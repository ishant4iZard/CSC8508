#include "Singleton.h"

template<class T>
void Singleton<T>::Create()
{
	if (intance == NULL)
	{
		intance = new T();
	}
}

template<class T>
void Singleton<T>::Destroy()
{
	if (intance != NULL)
	{
		delete intance;
		intance = NULL;
	}
}

template<class T>
T& Singleton<T>::Instance()
{
	if (intance == NULL)
	{
		Create();
	}
	return *intance;
}