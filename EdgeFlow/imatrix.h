#ifndef _IMATRIX_H_
#define _IMATRIX_H_
#include<iostream>
using namespace std;

class imatrix {
private:

public:
	enum num_type {INT, FLOAT};
	int Nr, Nc;
	int** p; 
	float** f;
	bool is_init;
	num_type type;
	
	//add by me 2014-5-5
	int** a;
	int** b;
	int** L;

	void delete_all() {
		if (type == INT)
		{
			for (int i = 0; i < Nr; i++)
				delete[] p[i];
			delete[] p;
		}
		else if (type == FLOAT)
		{
			for (int i = 0; i < Nr; i++)
				delete[] f[i];
			delete[] f;
		}
	}
	imatrix() 
    {
		is_init = true;
		type = INT;
		Nr = 1, Nc = 1;
		p = new int*[Nr];
		for(int i = 0; i < Nr; i++)
		   p[i] = new int[Nc];
        p[0][0]=1; 
    };
	imatrix(int i, int j) 
    {
		type = INT;
		is_init = true;
		Nr = i, Nc = j;
		
		p = new int*[Nr];
		for(i = 0; i < Nr; i++)
		   p[i] = new int[Nc];
    };
	imatrix(imatrix& b) {
		type = INT;
		is_init = true;
		Nr = b.Nr;
		Nc = b.Nc;
		p = new int*[Nr];
		for (int i = 0; i < Nr; i++) {
			p[i] = new int[Nc];
			for (int j = 0; j < Nc; j++) {
				p[i][j] = b[i][j];
			}
		}
	}

	imatrix(int i, int j, num_type type)
	{
		is_init = true;
		if (type == INT)
		{
			this->type = INT;
			Nr = i, Nc = j;

			p = new int* [Nr];
			for (i = 0; i < Nr; i++)
				p[i] = new int[Nc];
		}
		else if (type == FLOAT)
		{
			this->type = FLOAT;
			Nr = i, Nc = j;

			f = new float* [Nr];
			for (i = 0; i < Nr; i++)
				f[i] = new float[Nc];
		}
	}
	void init(int i, int j)
	{
		if (is_init)
		{
			delete_all();
		}
		Nr = i, Nc = j;
		p = new int*[Nr];
		for(i = 0; i < Nr; i++)
		   p[i] = new int[Nc];
    };

	~imatrix()
	{
		delete_all();
	}
	int* operator[](int i) { return p[i]; };

	int& get( int i, int j ) const { return p[i][j]; }
	int getRow() const { return Nr; }
	int getCol() const { return Nc; }
	
	void zero()
	{
		for (int i = 0; i < Nr; i++) 
			for (int j = 0; j < Nc; j++) 
				p[i][j] = 0;
	}
	void copy(imatrix& b)
	{
		init(b.Nr, b.Nc);
		for (int i = 0; i < Nr; i++) 
			for (int j = 0; j < Nc; j++) 
				p[i][j] = b.p[i][j];
	}

	void print()
	{
		for (int i = 0; i < Nr; i++)
		{
			for (int k = 0; k < Nc; k++)
			{
				cout << p[i][k] << " ";
			}
			cout << endl;
		}
	}
};


#endif
