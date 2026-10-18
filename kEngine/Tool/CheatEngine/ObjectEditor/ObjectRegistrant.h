#pragma once
#include <vector>

/// ObjectEditorにObjectを登録するためのクラス

class Object;
class ObjectRegistrant
{
public:
	static ObjectRegistrant& GetInstance();

	void Register(Object* obj);
	void Unregister(Object* obj);
	const std::vector<Object*>& GetObjects() const { return objects_; }

private:
	std::vector<Object*> objects_;
};

