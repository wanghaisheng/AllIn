#ifndef _H_SIGLETON_
#define _H_SIGLETON_
#pragma once
template <typename T>
class Singleton
{
private:    Singleton() {}  
			Singleton(const Singleton& rhs);  
			Singleton& operator=(const Singleton& rhs);
public:   
	static T * Instance()   
	{      
		static T * pT = NULL;   
		if (!pT)      
		{       
			static T instance;     
			pT = &instance;       
		}      
		return pT;    
	}};
#endif