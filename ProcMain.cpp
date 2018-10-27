#include "analyseXml.h"
#include "procMain.h"


extern stringstream framess;
extern ofstream logFile;
extern string logname;
extern stringstream frameobj;
extern stringstream totalFrame;

extern vector<string> xmlContent;	// ���xml��Ӧ��ʾ������
extern vector<string> addInXml;	// ���ÿ��obj�е�transfrom����
extern vector<string> abcProp;		// ���ÿ��abc�����ԣ��ļ������⣩
extern size_t tabNum;	// ���������Ʊ�������ڿ��Ƹ�ʽ

//����д֡�ļ�����Ϣ��һ���ж���֡��ÿһ֡xml�ļ���Ӧ��·����
ofstream infofile;
string infoname;




//����ÿһ֡��xml�Լ�obj�ļ�
void generatexml(string xmlPath,double frame1)
{
	objCount=0;
	objPath=xmlPath.substr(0,xmlPath.find_last_of("\\"))+"\\";

	char tmp[40];
    time_t t=time(0);
    strftime( tmp, sizeof(tmp), "%Y-%m-%d %X",localtime(&t) );

	frameobj.clear();
	frameobj.str("");
	frameobj<<frame1;

	//��������ʼ����ӡlog
	logname=objPath+frameobj.str()+".log";
	

	logFile.open((logname).c_str());
	logFile<<tmp<<"\tstart"<< '\n';

	string xmlFile;
	xmlFile=xmlPath;
	cout<<xmlFile<<'\n';

	frame=frame1;
	cout<<frame<<'\n';//��������Obj�ļ���֡��

	xmlAnalyze(xmlFile);


	framess.clear();
	framess.str("");
	framess<<frame;
	string framename=objPath+"frame"+framess.str()+".xml";
	//cout<<"framename:"<<framename<<endl;
	ofstream fout(framename.c_str());
	for (size_t i = 0; i != xmlContent.size(); ++i) 
		fout << xmlContent[i] << '\n';


	infofile.open(infoname.c_str(),ios::app);
	infofile<<framename<<'\n';
	infofile.close();

	

	char tmpfinal[40];
    time_t tfinal=time(0);
    strftime( tmpfinal, sizeof(tmpfinal), "%Y-%m-%d %X",localtime(&tfinal) );
    logFile<<tmpfinal<<"\tcompleted"<<'\n';
    logFile.close();

}

////ʹ��������
int main(int argc,char** argv)
{
	generatexml(argv[1],atoi(argv[2]));
}

