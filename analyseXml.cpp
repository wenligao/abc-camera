#include "analyseXml.h"


//ȫ�ֱ���������

extern stringstream framess;
extern ofstream logFile;
extern stringstream frameobj;
extern stringstream totalFrame;

// chen
vector<string> xmlContent;	// ���xml��Ӧ��ʾ������
vector<string> addInXml;	// ���ÿ��obj�е�transfrom����
vector<string> abcProp;		// ���ÿ��abc�����ԣ��ļ������⣩
size_t tabNum;	// ���������Ʊ�������ڿ��Ƹ�ʽ
bool hasOutputCamera;



//walk,visit�����ǲο�abcImPort�ķ���

void walk(Alembic::Abc::IArchive & iRoot)
{
	IObject mParent = iRoot.getTop();


	if (!iRoot.valid())
	{
		cout << "�ĵ���Ч��û�и��ڵ㣡" << '\n';
		exit(0);
	}
	//Ԥ���ػ����νṹ
	AlembicObjectPtr topObject =
		previsit(AlembicObjectPtr(new AlembicObject(iRoot.getTop())));

	if (!topObject)
		cout << "topObjectΪ��" << '\n';

	size_t numChildren = topObject->getNumChildren();
	if (numChildren == 0)
	{
		//cout << "numChildrenΪ��" << endl;
	}

	for (size_t i = 0; i < numChildren; i++)
	{	
		visit(topObject->getChild(i));
	}
}

//�ֶ���д����
void subdivide(const string &filePath) // chen
{
	try
	{
		Alembic::AbcCoreFactory::IFactory factory;
		IArchive archive = factory.getArchive(filePath);
		walk(archive);
	}//try�����ĵط�
	catch (const exception & e)
	{
		cerr << e.what() << '\n';
		cerr << "AlembicRiProcedural: cannot read arguments file: ";
		cerr << filePath << '\n';
	}
}


void xmlAnalyze(const string &xmlFile)
{
	int abcNum=0;
	ifstream fin(xmlFile.c_str());
	if(!fin)
	{
		cout<<"û���ҵ��ļ���\n";
		exit(0);
	}
	string line; 

	//����Ĵ�����ѭ��������abc��ǩ
	while (getline(fin, line)) {
		////����abc��ǩ
		//���û���ҵ�abc��ǩ
		if(line.find("<shape type=\"abc\">") == string::npos)
		{
			xmlContent.push_back(line);
		}else {
			string abcFile;
			abcProp.clear();
			abcNum++;
			tabNum = line.find("<shape type=\"abc\">");
			while (getline(fin, line) && line.find("</shape>") == string::npos) {
				if (line.find("name=\"filename\"") != string::npos) {
					size_t pos = line.find("value");
					abcFile = line.substr(pos+7, line.size()-pos-10);
					cout<<"abcfile:"<<abcFile<<'\n';
				}	
				else
					abcProp.push_back(line);//��abc��ǩ�е����ּ���������	
			}
	
			subdivide(objPath+abcFile);//���·��,�Լ�����objPath�������ҵ���ÿһ��abc�ļ���
			//subdivide(abcFile);
		}


		////��������ı�ǩ
		if(line.find("<sensorabc type=\"abc\">") != string::npos)
		{
			xmlContent.pop_back();
			string abcFile;
			abcProp.clear();
			abcNum++;
			tabNum = line.find("<sensorabc type=\"abc\">");
			hasOutputCamera=true;
			while (getline(fin, line) && line.find("</sensorabc>") == string::npos) {
				if (line.find("name=\"filename\"") != string::npos) {
					size_t pos = line.find("value");
					abcFile = line.substr(pos+7, line.size()-pos-10); 
					cout<<"abcfile:"<<abcFile<<'\n';
				}	
				else
					abcProp.push_back(line);//��abc��ǩ�е����ּ���������	
			}
	
			subdivide(objPath+abcFile);//���·��,�Լ�����objPath�������ҵ���ÿһ��abc�ļ���
			//subdivide(abcFile);

			//д�������Ϣ(���Ҫ�������棬���������д���)
	        writeSensor();

			//д��max�������Ϣ
			writeSensorMax();
		}



	}//whileѭ������

	
	//���xml�ļ���û��abc��ǩ��������˳�
	if(abcNum==0)
	{
		cout<<"û��abc�ļ��������˳�"<<'\n';
		exit(0);
	}
}
