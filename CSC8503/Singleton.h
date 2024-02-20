#pragma once

template <class T>
class Singleton
{
public:
	static void Create();
	static void Destroy();

	static T& Instance();
protected:
	Singleton(void) {}
	virtual ~Singleton(void) {}

	static T* intance;
};

template<class T>
T* Singleton<T>::intance = nullptr;