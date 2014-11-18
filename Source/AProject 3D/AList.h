#ifndef __APROJECT_GENERIC_LIST__
#define __APROJECT_GENERIC_LIST__

template <class Type>
class AList
{
public:
	AList (int s = 0);
	~AList ();

public:
	Type*	element;
	int		num;
	int		array_size;

public:
	void	allocate(int s);
	void	SetSize(int s);
	void	Pack();
	void	Add(Type);
	void	AddUnique(Type);
	int 	Contains(Type);
	void	Remove(Type);
	void	DelIndex(int i);

public:
	Type	&operator[](int i){assert(i>=0 && i<num); return element[i];}
};


template <class Type>
AList<Type>::AList(int s)
{
	num = 0;
	array_size = 0;
	element = NULL;
	if(s)
		allocate (s);
}

template <class Type>
AList<Type>::~AList()
{
	delete element;
}

template <class Type>
void AList<Type>::allocate(int s)
{
	assert(s > 0);
	assert(s >= num);

	Type *old = element;
	array_size = s;
	element = new Type [array_size];
	assert(element);

	for(int i = 0; i < num; i ++)
		element [i] = old [i];
		
	SAFE_DELETE(old);
}

template <class Type>
void AList<Type>::SetSize(int s)
{
	if (s == 0)
	{
		SAFE_DELETE(element);
	}
	else
	{
		allocate(s);
	}
	num = s;
}

template <class Type>
void AList<Type>::Pack()
{
	allocate(num);
}

template <class Type>
void AList<Type>::Add(Type t)
{
	assert(num <= array_size);
	if(num == array_size)
	{
		allocate((array_size)?array_size*2:16);
	}
	element[num++] = t;
}

template <class Type>
int AList<Type>::Contains(Type t)
{
	int i;
	int count = 0;
	for(i = 0; i < num; i ++)
	{
		if(element[i] == t) count++;
	}
	return count;
}

template <class Type>
void AList<Type>::AddUnique(Type t)
{
	if(!Contains(t)) Add(t);
}


template <class Type>
void AList<Type>::DelIndex(int i)
{
	assert(i < num);
	num --;

	while(i < num)
	{
		element [i] = element [i+1];
		i++;
	}
}

template <class Type>
void AList<Type>::Remove(Type t)
{
	int i;
	for(i = 0; i < num; i ++)
	{
		if(element[i] == t)
			break;
	}
	DelIndex(i);

	for(i = 0; i < num; i ++)
	{
		assert(element[i] != t);
	}
}

#endif