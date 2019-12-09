
#include "stdafx.h"

#ifdef __linux__
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#endif
#include <thread>
#include <wchar.h>
#include <string>
#include "AddInNative.h"
#include <Windows.h>
#include <fstream>
#include <string>

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qmessagebox.h>//	#include <QMessageBox>
int argc = 0;
char **argv = 0;
static QApplication *a=new QApplication(argc, argv);

#include "QtCore/qdatetime.h"
#include <QtCore/qstring.h>
#include <QtCore/qdir.h>
#include <QtCore/qthread.h>
void myLog(std::string text) {
	std::ofstream outfile;

	QString time_format = "yyyy-MM-dd  HH:mm:ss";
	QDateTime a = QDateTime::currentDateTime();
	QString as = a.toString(time_format);

	outfile.open("F:/log.txt", std::ios_base::app);
	outfile << as.toStdString()<<" | "<< text << std::endl;
	outfile.close();
}

static const wchar_t g_kClassNames[] = L"CAddInNative"; //|OtherClass1|OtherClass2";
static IAddInDefBase *pAsyncEvent = NULL;
uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len = 0);
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
uint32_t getLenShortWcharStr(const WCHAR_T* Source);



#include "QtMultimedia/qcamera.h" //#include <QCamera> 
#include "QtMultimedia/qcameraimagecapture.h" //#include <QCameraImageCapture>
#include "QtMultimedia/qcamerainfo.h"//#include <QCameraInfo>
#include "QtCore/qbuffer.h" //#include <QBuffer>
#include <QtCore/qtimer.h>

#include "QtGui/qimage.h"//#include <QImage>


//static QCamera *cam = new QCamera;

//---------------------------------------------------------------------------//
long GetClassObject(const wchar_t* wsName, IComponentBase** pInterface)
{
	myLog("GetClassObject");
    if(!*pInterface)
    {
        *pInterface= new CAddInNative();
        return (long)*pInterface;
    }
	myLog("GetClassObject bad end");
    return 0;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
	myLog("DestroyObject");
   if(!*pIntf)
      return -1;

   delete *pIntf;
   *pIntf = 0;
   return 0;
}
//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
	myLog("GetClassNames");
    //static WCHAR_T* names = 0;
    /*if (!names)
        ::convToShortWchar(&names, g_kClassNames);*/
	WCHAR_T* names = L"CAddInNative";
    return names;

}

//void func(){
//		if (!cam->isAvailable()) {
//			QMessageBox msgBox;
//			QString text = "Камера отвалилась. Необходим перезапуск. Camera not available.";
//			msgBox.setText(text);
//			msgBox.exec();
//		}
//		myLog("singleShot");
//		QTimer::singleShot(2000, func);
//};
// C:\Qt\Qt5.13.2\5.13.2\msvc2017_64\bin\moc.exe AddInNative.cpp -o handleMoc.h
//class MyThread : public QThread
//{
//	Q_OBJECT
//public:
//	MyThread(QCamera *cam) :cam(cam) {}
//	QCamera *cam;
//	void run1();
//};


//void MyThread::run1()
//{
//	myLog("MyThread::run() begin");
//	/*while (true) {
//		myLog("MyThread::run()");
//		if (!cam->isAvailable()) {
//			QMessageBox msgBox;
//			msgBox.setText("Camera not available.");
//			msgBox.exec();
//		}
//		QThread::sleep(1);
//	}*/
//}

void CAddInNative::beginGivesMePhoto()
{
	QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
	if (0 == cameras.size()) {
		myLog("msgBox begin");
		QMessageBox msgBox;
		msgBox.setText("Not found camera");
		msgBox.exec();
		myLog("msgBox end");
		return;
	}
	QCamera *cam = new QCamera;
	cam->setCaptureMode(QCamera::CaptureStillImage);
	QCameraImageCapture *cap = new QCameraImageCapture(cam);
	cap->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
	QObject::connect(cap, &QCameraImageCapture::imageCaptured, [=](int id, QImage img) {
		int static counter = 0;
		QString path = "C:/imgs/";
		QDir dir(path);
		if (!dir.exists()) {
			dir.mkdir(".");
		}
		QString filePath = path + QString("").setNum(counter) + ".png";
		if (!img.save(filePath, "PNG")) {
			QMessageBox msgBox;
			msgBox.setText("Can not save image:""" + filePath + """.");
			msgBox.exec();
		}
		myLog("Good save");
		++counter;
		if (counter > 100) {
			counter = 0;
		}
		wchar_t *who = L"ComponentNative", *what = L"Timer";
		wchar_t wstime[300];
		int size=filePath.toWCharArray(wstime);
		wstime[size] = '\0';
		pAsyncEvent->ExternalEvent(who, what, wstime);
	});

	QObject::connect(cap, &QCameraImageCapture::readyForCaptureChanged, [=](bool state) {
		if (state == true) {
			cam->searchAndLock();
			cap->capture();
			cam->unlock();
		}
	});

	QObject::connect(cap, QOverload<int, QCameraImageCapture::Error, const QString &>::of(&QCameraImageCapture::error),
		[=](int id, QCameraImageCapture::Error error, const QString &errorString) {
		QMessageBox msgBox;
		QString text = "Error with CameraImageCapture:" + errorString;
		switch (error)
		{
		case QCameraImageCapture::NoError:
			text += "No Errors.;";
			break;
		case QCameraImageCapture::NotReadyError:
			text += "The service is not ready for capture yet.;";
			break;
		case QCameraImageCapture::ResourceError:
			text += "Device is not ready or not available.;";
			break;
		case QCameraImageCapture::OutOfSpaceError:
			text += "No space left on device.;";
			break;
		case QCameraImageCapture::NotSupportedFeatureError:
			text += "Device does not support stillimages capture.;";
			break;
		case QCameraImageCapture::FormatError:
			text += "Current format is not supported.;";
			break;
		}
		msgBox.setText(text);
		msgBox.exec();
		exit(1);
	});
	QObject::connect(cam, QOverload<QCamera::Error>::of(&QCamera::error),
		[=](QCamera::Error value) {
		QMessageBox msgBox;
		QString text = "Camera error. Необходим перезапуск.";
		switch (value)
		{
		case QCamera::NoError:
			text += "Ошибок нет.";
			break;
		case QCamera::CameraError:
			text += "Какая то ошибка с камерой. Точно определить не удается. An error has occurred.";
			break;
		case QCamera::InvalidRequestError:
			text += "System resource doesn't support requested functionality.";
			break;
		case QCamera::ServiceMissingError:
			text += "No camera service available.";
			break;
		case QCamera::NotSupportedFeatureError:
			text += "The feature is not supported.";
			break;
		}
		msgBox.setText(text);
		msgBox.exec();
	});

	cam->start();

	/*QTimer timer;
	timer.setInterval(1000);
	QObject::connect(&timer, &QTimer::timeout, [=]() {
		if (!cam->isAvailable()) {
			QMessageBox msgBox;
			QString text = "Камера отвалилась. Необходим перезапуск. Camera not available.";
			msgBox.setText(text);
			msgBox.exec();
		}
		myLog("Inline timer");
	});
	timer.start();*/
	//MyThread thread(cam);
	//thread.start(QThread::Priority::LowPriority);
	auto func = [=](QCamera *local_cam)
	{
		
		myLog("func thread");
		while (true) {
			if (!local_cam->isAvailable()) {
				MessageBox(
					NULL,
					"Camera not available.",
					"Camera error",
					MB_OK
				);
				return 0;
			}
			else {
				QThread::sleep(1);
			}
			
		}
	};

	std::thread th(func, cam);
	th.detach();
}



//---------------------------------------------------------------------------//
//CAddInNative
CAddInNative::CAddInNative()
{
	myLog("CAddInNative");
}
//---------------------------------------------------------------------------//
CAddInNative::~CAddInNative()
{
	myLog("~CAddInNative");
}
//---------------------------------------------------------------------------//
bool CAddInNative::Init(void* pConnection)
{ 
	myLog("Init");
	IAddInDefBase_point = (IDispatch *)pConnection;
	pAsyncEvent = (IAddInDefBase*)pConnection;
	return true;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetInfo()
{ 
	myLog("GetInfo");
    return 2000; 
}
//---------------------------------------------------------------------------//
void CAddInNative::Done()
{
	myLog("Done");
}
//---------------------------------------------------------------------------//
bool CAddInNative::RegisterExtensionAs(WCHAR_T** wsLanguageExt)
{ 
	myLog("RegisterExtensionAs");
	wchar_t *wsExtension = L"VkForCameraThis";
	int iActualSize = ::wcslen(wsExtension) + 1;
	WCHAR_T* dest = 0;
	if (this->memManager)
	{
		auto res=this->memManager->AllocMemory((void**)wsLanguageExt, iActualSize * sizeof(WCHAR_T));
		if (res)
			::convToShortWchar(wsLanguageExt, wsExtension, iActualSize);
		myLog("RegisterExtensionAs return 'VkForCameraThis'");
		return true;
	}
	myLog("RegisterExtensionAs bad end");
	return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNProps()
{ 
	myLog("GetNProps");
    return eLastProp;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindProp(const WCHAR_T* wsPropName)
{ 
	myLog("FindProp");
    return -1;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetPropName(long lPropNum, long lPropAlias)
{ 
	myLog("GetPropName");
    return 0;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{ 
	myLog("GetPropVal");
    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::SetPropVal(const long lPropNum, tVariant* varPropVal)
{ 
	myLog("SetPropVal");
    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropReadable(const long lPropNum)
{ 
	myLog("IsPropReadable");
    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropWritable(const long lPropNum)
{
	myLog("IsPropWritable");
	return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNMethods()
{ 
	myLog("GetNMethods"); 
	return eLastMethod;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindMethod(const WCHAR_T* wsMethodName)
{ 
	myLog("FindMethod"); 
	wchar_t* tempString = 0;
	convFromShortWchar(&tempString, wsMethodName);
	std::wstring wstring(tempString);
	std::string methodMame(wstring.begin(), wstring.end());
	myLog("FindMethod get:"+ methodMame);
	if ("beginGiveMePhoto" == methodMame) {
		int result = CAddInNative::Methods::beginGiveMePhoto;
		myLog("FindMethod return:"+QString("").setNum(result).toStdString());
		return result;
	}
	return -1;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetMethodName(const long lMethodNum, 
                            const long lMethodAlias)
{ 
	myLog("GetMethodName"); 
	return 0;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNParams(const long lMethodNum)
{ 
	myLog("GetNParams:"+ QString("").setNum(lMethodNum).toStdString());
	switch (lMethodNum)
	{
	case CAddInNative::Methods::beginGiveMePhoto:
		return 0;
	}
	return 0;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetParamDefValue(const long lMethodNum, const long lParamNum,
                          tVariant *pvarParamDefValue)
{ 
	myLog("GetParamDefValue"); 
	return false;
} 
//---------------------------------------------------------------------------//
bool CAddInNative::HasRetVal(const long lMethodNum)
{ 
	myLog("HasRetVal:"+QString("").setNum(lMethodNum).toStdString());
	switch (lMethodNum)
	{
	case CAddInNative::CAddInNative::beginGiveMePhoto:
		return false;
	}
	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsProc(const long lMethodNum,
                    tVariant* paParams, const long lSizeArray)
{ 
	myLog("CallAsProc get:"+QString("").setNum(lMethodNum).toStdString());
	switch (lMethodNum)
	{
	case CAddInNative::Methods::beginGiveMePhoto:
		myLog("CallAsProc need call 'beginGiveMePhoto'.");
		this->beginGivesMePhoto();
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsFunc(const long lMethodNum,
                tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{ 
	myLog("CallAsFunc"); 
	return false;
}
//---------------------------------------------------------------------------//
void CAddInNative::SetLocale(const WCHAR_T* loc)
{
	myLog("SetLocale");
#ifndef __linux__
    _wsetlocale(LC_ALL, loc);
#else
    int size = 0;
    char *mbstr = 0;
    wchar_t *tmpLoc = 0;
    convFromShortWchar(&tmpLoc, loc);
    size = wcstombs(0, tmpLoc, 0)+1;
    mbstr = new char[size];

    if (!mbstr)
    {
        delete[] tmpLoc;
        return;
    }

    memset(mbstr, 0, size);
    size = wcstombs(mbstr, tmpLoc, wcslen(tmpLoc));
    setlocale(LC_ALL, mbstr);
    delete[] tmpLoc;
    delete[] mbstr;
#endif
}
//---------------------------------------------------------------------------//
bool CAddInNative::setMemManager(void* mem)
{
	myLog("setMemManager");
	memManager = (IMemoryManager*)mem;
    return true;
}
//---------------------------------------------------------------------------//
uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len)
{
	if (!len)
        len = ::wcslen(Source)+1;

    if (!*Dest)
        *Dest = new WCHAR_T[len];

    WCHAR_T* tmpShort = *Dest;
    wchar_t* tmpWChar = (wchar_t*) Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len*sizeof(WCHAR_T));
    do
    {
        *tmpShort++ = (WCHAR_T)*tmpWChar++;
        ++res;
    }
    while (len-- && *tmpWChar);

    return res;
}
//---------------------------------------------------------------------------//
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
{
    if (!len)
        len = getLenShortWcharStr(Source)+1;

    if (!*Dest)
        *Dest = new wchar_t[len];

    wchar_t* tmpWChar = *Dest;
    WCHAR_T* tmpShort = (WCHAR_T*)Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len*sizeof(wchar_t));
    do
    {
        *tmpWChar++ = (wchar_t)*tmpShort++;
        ++res;
    }
    while (len-- && *tmpShort);

    return res;
}
//---------------------------------------------------------------------------//
uint32_t getLenShortWcharStr(const WCHAR_T* Source)
{
    uint32_t res = 0;
    WCHAR_T *tmpShort = (WCHAR_T*)Source;

    while (*tmpShort++)
        ++res;

    return res;
}
//---------------------------------------------------------------------------//
