#pragma once
#ifndef PHYSICAL_OBJECT_SENTRY
#define PHYSICAL_OBJECT_SENTRY

#include "geometry.h"
#include "semaphore.h"
#include <optional>
#include <future>
#include <list>
#include <stdint.h>

class PhysicalObject {
protected:
  float x_frac = 0, y_frac = 0;
  float x_vel = 0, y_vel = 0;
  float x_acc = 0, y_acc = 0;
  std::optional<Box2d> place_to_live = std::nullopt;
public:
  PhysicalObject() = default;
  PhysicalObject(std::optional<Box2d> ptl): place_to_live(std::move(ptl)) {}
  virtual void draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) {
    (void)buffer; (void)screen_h; (void)screen_w;
  }
  virtual void act(float dt);
  virtual bool isInsideBox(const Box2d& box);

  virtual ~PhysicalObject() {};
}; // PhysicalObject

using PhysicalObjectHandle = std::unique_ptr<PhysicalObject>;


class PhysicalObjectManager {
public:
  struct SemaphoreManager {
    Semaphore payload_semaphore;
    Semaphore can_remove_semaphore;
    SemaphoreManager(int payload_count = 1, int remove_count=1)
      : payload_semaphore(payload_count), can_remove_semaphore(remove_count) {}
  };
  using SemaphoreIterator = std::list<SemaphoreManager>::iterator;
  class ObjectSmartHandle {
    PhysicalObjectHandle ptr;
    std::optional<SemaphoreIterator> semaphore_iter = std::nullopt;
    void decSemaphore() {
      if (semaphore_iter) {
        // this section is hopefully fine
        // shouldn't be called if payload_semaphore is not locked
        (*semaphore_iter)->payload_semaphore.decUnsafe();
        (*semaphore_iter)->can_remove_semaphore.dec();
      }
    }
  public:
    ObjectSmartHandle(PhysicalObjectHandle&& h, std::optional<SemaphoreIterator> i = std::nullopt)
      : ptr(std::move(h)), semaphore_iter(i) {}
    ~ObjectSmartHandle() { decSemaphore(); }
    ObjectSmartHandle& operator=(ObjectSmartHandle&& other) {
      ptr = std::move(other.ptr);
      decSemaphore();
      semaphore_iter = other.semaphore_iter;
      other.semaphore_iter = std::nullopt;
      return *this;
    }
    PhysicalObjectHandle& operator->() { return ptr; }
    const PhysicalObjectHandle& operator->() const { return ptr; }
    PhysicalObject& operator*() { return *ptr; }
    const PhysicalObject& operator*() const { return *ptr; }
    std::optional<SemaphoreIterator>& getSemaphore() { return semaphore_iter; }
    const std::optional<SemaphoreIterator>& getSemaphore() const { return semaphore_iter; }
    friend PhysicalObjectManager;
  };
  using ObjectIterator = std::list<ObjectSmartHandle>::iterator;
  using ObjectConstIterator = std::list<ObjectSmartHandle>::const_iterator;
  using ObjectInsertIterator = std::insert_iterator<std::list<ObjectSmartHandle>>;
  using FutureIterator = std::list<std::future<void>>::iterator;
  using FutureConstIterator = std::list<std::future<void>>::const_iterator;
  using FutureInsertIterator = std::insert_iterator<std::list<std::future<void>>>;
private:
  std::list<ObjectSmartHandle> physical_objects;
  std::list<SemaphoreManager> semaphores;
  std::list<std::future<void>> futures;
public:
  ObjectIterator objectsBegin() { return physical_objects.begin(); }
  ObjectIterator objectsEnd() { return physical_objects.end(); }
  ObjectConstIterator objectsBegin() const { return physical_objects.begin(); }
  ObjectConstIterator objectsEnd() const { return physical_objects.end(); }
  ObjectIterator eraseObjectUnsafe(ObjectIterator iter) {
    return physical_objects.erase(iter);
  }
  ObjectIterator eraseObject(ObjectIterator iter) {
    if (iter->semaphore_iter) {
      ObjectIterator res = iter; // temporarily for initialization purposes
      {
        auto l = (*iter->semaphore_iter)->payload_semaphore.lock();
        res = eraseObjectUnsafe(iter); // real deal
        // l.~Lock();
      }
      return res;
    } else {
      return eraseObjectUnsafe(iter);
    }
  }
  // ObjectIterator appendObject(PhysicalObjectHandle&& obj) { return physical_objects.insert(objectsEnd(), std::move(obj)); }
  // ObjectIterator prependObject(PhysicalObjectHandle&& obj) { return physical_objects.insert(objectsBegin(), std::move(obj)); }
  ObjectIterator appendObject(PhysicalObjectHandle&& obj) { return physical_objects.emplace(objectsEnd(), std::move(obj)); }
  ObjectIterator prependObject(PhysicalObjectHandle&& obj) { return physical_objects.emplace(objectsBegin(), std::move(obj)); }
  ObjectInsertIterator objectAppender() { return ObjectInsertIterator(physical_objects, objectsEnd()); }
  ObjectInsertIterator objectPrepender() { return ObjectInsertIterator(physical_objects, objectsBegin()); }
  SemaphoreIterator semaphoresBegin() { return semaphores.begin(); }
  SemaphoreIterator semaphoresEnd() { return semaphores.end(); }
  SemaphoreIterator eraseSemaphoreUnsafe(SemaphoreIterator iter) {
    return semaphores.erase(iter);
  }
  std::optional<SemaphoreIterator> eraseSemaphore(SemaphoreIterator iter) {
    auto l = iter->can_remove_semaphore.lock();
    if (iter->can_remove_semaphore.countUnsafe() == 0) {
      return eraseSemaphoreUnsafe(iter);
    } else {
      return std::nullopt;
    }
  }
  SemaphoreIterator addSemaphore(int payload_count=1, int remove_count=1) {
    return semaphores.emplace(semaphoresEnd(), payload_count, remove_count);
  };
  ObjectIterator appendSemaphoredObject(PhysicalObjectHandle&& obj) {
    auto semaphore_iter = addSemaphore();
    // return physical_objects.insert(objectsEnd(), ObjectSmartHandle(std::move(obj), semaphore_iter));
    auto res = physical_objects.emplace(objectsEnd(), std::move(obj), semaphore_iter);
    res->semaphore_iter = semaphore_iter;
    return res;
  }
  ObjectIterator prependSemaphoredObject(PhysicalObjectHandle&& obj) {
    auto semaphore_iter = addSemaphore();
    // return physical_objects.insert(objectsEnd(), ObjectSmartHandle(std::move(obj), semaphore_iter));
    auto res = physical_objects.emplace(objectsBegin(), std::move(obj), semaphore_iter);
    res->semaphore_iter = semaphore_iter;
    return res;
  }
  FutureIterator futuresBegin() { return futures.begin(); }
  FutureIterator futuresEnd() { return futures.end(); }
  FutureConstIterator futuresBegin() const { return futures.begin(); }
  FutureConstIterator futuresEnd() const { return futures.end(); }
  FutureInsertIterator futureAppender() { return std::inserter(futures, futures.end()); }
  FutureIterator eraseFuture(FutureIterator iter) { return futures.erase(iter); }

  int objectCount() const { return physical_objects.size(); }
  int semaphoreCount() const { return semaphores.size(); }
  int futureCount() const { return futures.size(); }
};

#endif
