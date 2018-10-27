#include <fstream>
#include <iostream>
#include <time.h>
#include "global.h"
#include "animatePorcess.h"
#include "SampleUtil.h"
#include "analyseXml.h"
using namespace std;

ofstream logFile;
extern string logname;

// chen
extern vector<string> addInXml; 
extern vector<string> xmlContent;
extern vector<string> abcProp;
extern size_t tabNum;
extern stringstream ss;//��������obj�ļ�������Ŀ
extern stringstream framess;//��������֡����Ŀ
extern stringstream frameobj;//��������obj�ļ���֡����Ŀ
vector<string> cameraProp;	//�������
extern bool hasOutputCamera;

// ��xmlContent���ҵ��������Ϣ����������xmlContent�е�λ��
int findDefaultCamera(vector<string> &xmlContent)
{
	for (int i = 0; i != xmlContent.size(); ++i) {
		if (xmlContent[i].find("sensor") != string::npos) {
			while (i != xmlContent.size() && xmlContent[i].find("</sensor>") == string::npos) {
				if (xmlContent[i].find("lookat") != string::npos) 
					return i;
				++i;
			}

			cout << "δ�ҵ�</sensor>����δ�ҵ�lookat" << '\n';
			return -1;
		}
	}

	cout << "δ�ҵ�sensor��ǩ" << '\n';
	return -1;
}

//���԰��������Ϣд��
void writeSensor()
{
		// cout << "IPolyMesh" << endl;
		// ƥ��IPolyMeshʱ��˵������������ռ���������Ҫ��������xmlContent
		// ��Ҫע��һ���ʽ�ϵĸı�
		// ���⻹��Ҫע�����һ�μ���cameraProp�ıض���obj��Ϣ��Ҫ����ɾȥ
		if (hasOutputCamera && !cameraProp.empty()) {
			//cameraProp.pop_back();
			int index = findDefaultCamera(xmlContent);
			// �����Ϣ����Ϊ�գ���xml�б�����lookat��Ϣ
			if (!cameraProp.empty() && index != -1) {
				string cameraTransformType[] = { " origin=" , " target=" , " up=" };
				string toDelete[] = { "<translate x=" , "\" y=\"" , "\" z=\"" , "/>" };
				string camP("<lookat"), &defut(xmlContent[index]);
				for (int i = 0; i != cameraProp.size(); ++i) {
					string &t = cameraProp[i];
					t.erase(t.find(toDelete[0]), toDelete[0].size());	// ɾ��<translate x=
					t.replace(t.find(toDelete[1]), toDelete[1].size(), " ");	// ��" y="�滻�ɿո�
					t.replace(t.find(toDelete[2]), toDelete[2].size(), " ");	// ������ͬ��
					t.erase(t.find(toDelete[3]), toDelete[3].size());	// ���һ��eraseͬ��
					camP += (cameraTransformType[i] + t);
				}

				//�����������������Ҫ��Ĭ��ֵ�������
				if (cameraProp.size() == 1)
					camP += defut.substr(defut.find(cameraTransformType[1]));
				else if (cameraProp.size() == 2)
				{
					//camP += defut.substr(defut.find(cameraTransformType[2]));
					//���ֻ���������������Ĭ��up����Ϊ0 1.0 0

					camP += " up= \"0 1.0 0\"/>";
				}
				else
					camP += toDelete[3];
				defut = string(tabNum + 2, '\t') + camP;

				cout<<"camtype2==="<<cameraTransformType[2]<<endl;
				cout<<"camp====="<<camP<<endl;
				cout<<"defut===="<<defut<<endl;
			}
		}//�����if������
}


//Ԥ���ػ����ĵ�
AlembicObjectPtr previsit(AlembicObjectPtr iParentObject)
{
	Alembic::Abc::IObject parent = iParentObject->object();
	const string name = parent.getFullName().c_str();
	const size_t numChildren = parent.getNumChildren();
	
	for (size_t i = 0; i < numChildren; i++)
	{
		Alembic::Abc::IObject child = parent.getChild(i);
		AlembicObjectPtr childObject =
			previsit(AlembicObjectPtr(new AlembicObject(child)));

		if (childObject)
		{
			iParentObject->addChild(childObject);
		}
	}
	return iParentObject;
}

void visit(AlembicObjectPtr iObject)
{
	Alembic::Abc::IObject iObj = iObject->object();
	if (Alembic::AbcGeom::IXform::matches(iObj.getHeader()))
	{
		//��ÿ��ƥ��֮ǰ��Ҫ���
		addInXml.clear();
		Alembic::AbcGeom::IXform xform(iObj, Alembic::Abc::kWrapExisting);
	
		//����Ĵ����Ƕ�ȡ�ĵ���IXform�е���Ϣ(�ο�prman�еĴ���ProcessXform)
		Alembic::AbcGeom::IXformSchema &xformFS = xform.getSchema();//��ȡ�ö����Ӧ��ģʽ
		TimeSamplingPtr ts = xformFS.getTimeSampling();
		size_t xformSamps = xformFS.getNumSamples();
		//cout<<"numsamples:"<<xformSamps<<'\n';
	
		SampleTimeSet sampleTimes;
		GetRelevantSampleTimes(ts, xformSamps, sampleTimes);

		//cout<<"sampleTimes:"<<sampleTimes.size()<<'\n';
		bool multiSample = sampleTimes.size()>1;
		vector<XformSample> sampleVectors;
		sampleVectors.resize(sampleTimes.size());

		size_t sampleTimeIndex = 0;
		for (SampleTimeSet::iterator I = sampleTimes.begin();
			I != sampleTimes.end(); ++I, ++sampleTimeIndex)
		{
			ISampleSelector sampleSelector(*I);
			xformFS.get(sampleVectors[sampleTimeIndex], sampleSelector);

		}//forѭ������

		for (size_t i = 0, e = xformFS.getNumOps(); i < e; ++i)
		{
			if (multiSample)
			{
				WriteMotionBegin(sampleTimes);
				//cout<<"multiSampleΪTrue"<<'\n';
			}
			for (size_t j = 0; j < sampleVectors.size(); ++j)
			{
				XformOp &op = sampleVectors[j][i];
				//cout<<"op.getType()=="<<op.getType()<<endl;
				switch (op.getType())
				{
				case kScaleOperation:
					{
						V3d value = op.getScale();
						// chen
						ostringstream oss;
						oss << "<scale x=\"" << value.x << "\" y=\""<< value.y << "\" z=\"" << value.z <<"\"/>";
						//cout << "<scale x=\"" << value.x << "\" y=\""<< value.y << "\" z=\"" << value.z <<"\"/>"<<'\n';
						addInXml.push_back(oss.str());
					}
					break;

				case kTranslateOperation:
					{
						V3d value = op.getTranslate();
						// chen
						ostringstream oss;
						oss << "<translate x=\"" << value.x << "\" y=\""<< value.y << "\" z=\"" << value.z <<"\"/>";
						//cout << "<translate x=\"" << value.x << "\" y=\""<< value.y << "\" z=\"" << value.z <<"\"/>"<<'\n';
						addInXml.push_back(oss.str());
					}
					break;

				case kRotateOperation:
				case kRotateXOperation:
					{
						V3d axis = op.getAxis();
						float degrees = op.getAngle();

						// chen
						ostringstream oss;
						oss << "<rotate x=\"" << axis.x << "\" angle=\""<< degrees << "\"/>";
						//cout << "<rotate x=\"" << axis.x << "\" angle=\""<< degrees << "\"/>"<<'\n';
						addInXml.push_back(oss.str());
					}
					break;
				
				case kRotateYOperation:
					{
						V3d axis = op.getAxis();
						float degrees = op.getAngle();

						// chen
						ostringstream oss;
						oss << "<rotate y=\"" << axis.y << "\" angle=\""<< degrees << "\"/>";
						//cout << "<rotate y=\"" << axis.y << "\" angle=\""<< degrees << "\"/>"<<'\n';
						addInXml.push_back(oss.str());
					}
					break;
					
				case kRotateZOperation:
					{
						V3d axis = op.getAxis();
						float degrees = op.getAngle();

						// chen
						ostringstream oss;
						oss << "<rotate z=\"" << axis.z << "\" angle=\""<< degrees << "\"/>";
						//cout << "<rotate z=\"" << axis.z << "\" angle=\""<< degrees << "\"/>"<<'\n';
						addInXml.push_back(oss.str());
					}
					break;

				}//switch������
			}
		}

		// chen
		// �����δ�����ʱ������ӦĬ��������Ϣ��������ģ����Ĭ��Ϊ��һ�����
		// ���ǲ���������obj����ֱ��д��xml����Ϊ��ȷ������Ĳ������м���
		// ����Ҫ��һ��vector�ݴ棬���֮��ƥ�䵽��IPolyMesh����˵�����������������
		// �������ֻ�ռ���һ��transform��Ϣ
		//������������Ϣ���Ͱѱ任��Ϣ���뵽�������������
		//hasOutputCamera�Ǹ�ȫ�ֱ���������ǰ���ҵ���̬����ı�ǩʱ����Ҫ������Ϊtrue
		//�����������Maya������Ľṹ
		if (hasOutputCamera && !addInXml.empty())
		{
			cameraProp.push_back(addInXml[0]);
		}

		size_t numChildren = iObject->getNumChildren();
		for (size_t i = 0; i<numChildren; i++)
		{
			visit(iObject->getChild(i));
			
		}
	}
	//ѭ�����������еĽڵ�
	else if (Alembic::AbcGeom::ISubD::matches(iObj.getHeader()))
	{
		// chen
		//cout << "IsubD" << endl;
		Alembic::AbcGeom::ISubD mesh(iObj, Alembic::Abc::kWrapExisting);
		Alembic::AbcGeom::ISubDSchema subSche = mesh.getSchema();
	}
	else if (Alembic::AbcGeom::IPolyMesh::matches(iObj.getHeader()))
	{
		
		//������Ϊfalse�����治�����������Ϣ���о�����Ӳ���ûʲôӰ�죩
		hasOutputCamera=false;

		Alembic::AbcGeom::IPolyMesh mesh(iObj, Alembic::Abc::kWrapExisting);
		Alembic::AbcGeom::IPolyMeshSchema meshSche = mesh.getSchema();
		//cout<<"IPolyMesh\n";
		//����ͳ��obj������
		++objCount;
		ss.clear();
		ss.str("");
		ss<<objCount;

		char tmp[40];
		time_t t=time(0);
		strftime( tmp, sizeof(tmp), "%Y-%m-%d %X",localtime(&t) );
		//���������־��Ϣ
		cout<<"objCount="<<ss.str()<<'\n';
        logFile<<tmp<<"\tanalysing..."<<"\t"<<ss.str()<<'\n';

		string add(tabNum, '\t'), 
			addObj = "<string name=\"filename\" value=\""  +string("a")+frameobj.str()+"_"+ss.str()+ ".obj\"/>";
		xmlContent.push_back(add + "<shape type=\"obj\">");
		
		//����ӵĴ���
		if(!addInXml.empty())
		{
			xmlContent.push_back(add + '\t' + "<transform name=\"toWorld\">");

			//��̬���vector�ĳ���,����ֻ��obj�ı任��Ϣ�����ȥ
			for(int i = addInXml.size() - 1; i >= 0; --i)
				xmlContent.push_back(add + "\t\t" + addInXml[i]);

			xmlContent.push_back(add + '\t' + "</transform>");
		}

		xmlContent.insert(xmlContent.end(), abcProp.begin(), abcProp.end());
		xmlContent.push_back(add + '\t' + addObj);
		xmlContent.push_back(add + "</shape>");
		xmlContent.push_back(string());

		//�ο�prman������дpolymesh����Ϣ
		Alembic::AbcGeom::IPolyMeshSchema &ps = mesh.getSchema();
		TimeSamplingPtr ts = ps.getTimeSampling();


		SampleTimeSet sampleTimes;
		GetRelevantSampleTimes(ts, ps.getNumSamples(), sampleTimes);

		bool multisample = sampleTimes.size()>1;
		if (multisample)
		{
			WriteMotionBegin(sampleTimes);
		}
		//��������obj������
		int count=0;
		for (SampleTimeSet::iterator iter = sampleTimes.begin(); iter != sampleTimes.end(); ++iter)
		{
			ISampleSelector sampleSelector(*iter);
			IPolyMeshSchema::Sample sample = ps.getValue(sampleSelector);

			//��õ����Ϣ
			P3fArraySamplePtr point = sample.getPositions();
			//���ļ��Ĳ���
			ofstream outfile;
			outfile.open((objPath + "a"+frameobj.str()+"_"+ss.str() + ".obj").c_str());

			//�Ƚ���ǿ������ת����Ȼ���ٻ�ȡ�������
			float32_t *fpoint = (float32_t *)(point->getData());
			outfile << "v" << " ";
			int num = 0;
			for (int i = 0; i<3 * (point->size()); i++)
			{
				if (num != 0 && num % 3 == 0)
				{
					outfile << '\n';
					outfile << "v" << " ";
				}
				outfile << fixed << *fpoint << " ";
				fpoint++;
				num++;
			}
			outfile << '\n'; //���������Ҫendl������Ͳ�Ҫ��close��'\n'Ҫ��closeһ��ˢ�»�����
			outfile.close();

			////���Ի��uv��Ϣ
			IV2fGeomParam uvParam = ps.getUVsParam();
			setUVs(frame, uvParam, objPath + "a"+frameobj.str()+"_"+ss.str() + ".obj");
			outfile.close();

			//���Ի�÷�����Ϣ
			IN3fGeomParam nParam = ps.getNormalsParam();
			setPolyNormals(frame, nParam, objPath +"a"+frameobj.str()+"_"+ ss.str() + ".obj");
			outfile.close();

			//���Ի�����������Ϣ
		    fillTopology(frame, uvParam, nParam, sample.getFaceIndices(), sample.getFaceCounts(), objPath + "a"+frameobj.str()+"_"+ ss.str() + ".obj");
		}
	}
	else if (Alembic::AbcGeom::ICamera::matches(iObj.getHeader()))
	{
		// chen
		// cout << "ICamera" << endl;
		Alembic::AbcGeom::ICamera cam(iObj, Alembic::Abc::kWrapExisting);

		Alembic::AbcGeom::ICameraSchema camSche = cam.getSchema();

		camSche = cam.getSchema();
		Alembic::AbcGeom::CameraSample *camsample = new Alembic::AbcGeom::CameraSample();//��ȡ��ģʽ����������
		camsample = new Alembic::AbcGeom::CameraSample();
		camSche.get(*camsample);//��ȡ��������

		//cout<<"camera\n";
		//cout<<iObj.getHeader().getFullName()<<'\n';
		//getCamera(frame,camSche);
	}
	else if (Alembic::AbcGeom::ICurves::matches(iObj.getHeader()))
	{
		// chen
		//cout << "ICurves" << endl;
		Alembic::AbcGeom::ICurves curves(iObj, Alembic::Abc::kWrapExisting);
		Alembic::AbcGeom::ICurvesSchema curveSche = curves.getSchema();
		bool isConstant = curves.getSchema().isConstant();
		Alembic::AbcGeom::ICurvesSchema::Sample *cursamp = new Alembic::AbcGeom::ICurvesSchema::Sample();
		curveSche.get(*cursamp);
	}
	else if (Alembic::AbcGeom::INuPatch::matches(iObj.getHeader()))
	{
		// chen
		//cout << "INuPatch" << endl;
		Alembic::AbcGeom::INuPatch nurbs(iObj, Alembic::Abc::kWrapExisting);
		Alembic::AbcGeom::INuPatchSchema inuSche = nurbs.getSchema();
	}
	else if (Alembic::AbcGeom::IPoints::matches(iObj.getHeader()))
	{
		// chen
		//cout << "IPoints" << endl;
		Alembic::AbcGeom::IPoints pts(iObj, Alembic::Abc::kWrapExisting);
		Alembic::AbcGeom::IPointsSchema ipoSche = pts.getSchema();
	}
}