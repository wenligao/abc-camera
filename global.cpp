#include <string>
#include <iostream>
#include <vector>
#include <sstream>
using namespace std;
//��������ȫ�ֱ���

string objPath;//����������ɵ�Obj�ļ�
string xformpath;
//string xmlFile;
stringstream ss;//��������obj�ļ�����Ŀ
stringstream framess;//��������xml�ļ�����Ӧ��֡�����������ַ�����ת����
stringstream  frameobj;//ÿһ��obj����Ӧ��֡��
stringstream totalFrame;//�������������־�е���֡��
string logname ;


double frame;//ָ����ʼ֡
int objCount;//����ͳ�Ƴ����е�obj����Ŀ
