/**************************************************
 *编译g++ -I /usr/include/eigen3 eigen.cpp -o eigen
 *
 **************************************************/
#include <Eigen/Dense>
#include <iostream>

using namespace std;
using namespace Eigen;

class VAD{
public:
	//VAD();
	//~VAD();
	void test();
private:
	//VectorXd x;
	MatrixXi v;
	VectorXi bb;
	//int** bias;
	int bias[2][12];
};
/*
VAD::VAD(){
	bias = new int*(2);
	for (int i=0; i<2; i++){
		bias[i] = new int(12);
		for(int j=0; j<12; j++){
			bias[i][j] = 0;
		}
	}
}
VAD::~VAD(){
	for (int i=0; i<2; i++){
		delete [] bias[i];
	}
	delete [] bias;
}
*/
/**************************
void VAD::test()
{
	int num = 10;
	int sum = 0;
	VectorXd x = VectorXd::Zero(num);
	for(int i=0; i<num; i++)
	{
		x(i) = i*i;
		sum += x(i);
	}
	cout << sum << endl;
}
****************************/

void VAD::test()
{
	//RowVectorXi v = RowVectorXi::Zero(4); //加上这句时,map就出错
	/*
	eigen.cpp:38:21: error: conflicting declaration ‘Eigen::Map<Eigen::Matrix<int, 1, -1>, 0, Eigen::Stride<0, 0> > v’
  	Map<RowVectorXi> v(data,4);
                           ^
	eigen.cpp:36:14: note: previous declaration as ‘Eigen::RowVectorXi v’
  	RowVectorXi v = RowVectorXi::Zero(4);
              	    ^
	
	int kk = 4;
	int data[] = {0,1,2,3,4,5,6,7,8,9};
	Map<MatrixXi,0> v(data,kk,2);
	cout << v << "\n";

	MatrixXi tt(3,6);
	for(int i=0; i<3; i++)
	{
		for(int j=0; j<6; j++)
		{
			tt(i,j) = i*2+j;
		}
	}
	cout << tt << endl;
	
	VectorXi aa(5);
	for(int i=0; i<5; i++)
	{
		aa(i) = i*2;
	}
	cout << aa << endl;*/
	
	for (int i=0; i<2; i++){
		for(int j=0; j<12; j++){
			bias[i][j] = i*2+j;
		}
	}
	Map<VectorXi,0> bb(bias[0],12);
	cout << bb << endl;
	
}


int main()
{	
	VAD d;
	d.test();
	return 0;
}

