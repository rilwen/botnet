#include <gtest/gtest.h>
#include "simbo/managed_object.hpp"
#include "simbo/object_manager.hpp"

using namespace simbo;

// Testuje zar�wno ManagedObject jaki i ObjectManager.

class MockManager;

class MockObject : public ManagedObject<MockObject, MockManager> {

};

class MockManager: public ObjectManager<MockObject, MockManager> {
public:
	MockObject* add(MockObject* obj) {
		add_object(*this, std::unique_ptr<MockObject>(obj), Passkey<MockManager>());
		return obj;
	}
};

TEST(ManagedObject, set_manager) {
	MockManager manager;
	MockObject* object = manager.add(new MockObject());
	ASSERT_EQ(&manager, &object->get_manager());
	ASSERT_EQ(1, manager.get_objects().size());
	ASSERT_EQ(object, manager.get_objects()[0].get());
}

TEST(ManagedObject, add_object_throws) {
	MockManager manager;
	ASSERT_THROW(manager.add(nullptr), std::invalid_argument);
}
