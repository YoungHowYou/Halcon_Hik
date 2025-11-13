#include "Halcon.h"
#include "Halcon_Hik.h"
#include "HalconCpp.h"
#include "MvCameraControl.h"
#define H_HikCamera_TAG 0xC0FFEE20
#define H_HikCamera_SEM_TYPE "HikCamera"
using namespace HalconCpp;

extern "C"
{
    typedef struct
    {
        void *海康相机句柄;
    } HUserHandleData;

    static Herror HUserHandleDestructor(Hproc_handle ph, HUserHandleData *data)
    {
        int ret = MV_OK;
        if (data->海康相机句柄)
        {

            ret = MV_CC_CloseDevice(data->海康相机句柄);
            ret = MV_CC_DestroyHandle(data->海康相机句柄);
            data->海康相机句柄 = NULL;
        }
        else
        {
            return 正确;
        }
        return HFree(ph, data);
    }
    // 句柄类型描述符
    const HHandleInfo HandleTypeUser = HANDLE_INFO_INITIALIZER_NOSER(H_HikCamera_TAG, H_HikCamera_SEM_TYPE, HUserHandleDestructor, NULL, NULL);
}

struct MyContext
{
    int 分时频闪数;
    char 相机用户名[64];
    HTuple 海康相机采集队列; // 直接放对象

    MyContext(int flash, const char *name, const HTuple &queue)
        : 分时频闪数(flash), 海康相机采集队列(queue)
    {
        strcpy_s(相机用户名, name);
    }
};

void __stdcall 海康_回调取图函数(unsigned char *图像指针, MV_FRAME_OUT_INFO_EX *帧信息, void *用户自定义数据容器)
{

    HObject ho_Image;
    HTuple hv_MessageHandle;
    // MyContext *ctx = (MyContext *)用户自定义数据容器;
    MyContext *ctx = static_cast<MyContext *>(用户自定义数据容器);
    // int *用户自定义数据 = reinterpret_cast<int *>(用户自定义数据容器);
    if (帧信息->enPixelType == PixelType_Gvsp_Mono8)
    {
        GenImage1(&ho_Image, "byte", (帧信息->nWidth) * (ctx->分时频闪数), (帧信息->nHeight) / (ctx->分时频闪数), (__int64)图像指针);
    }
    else if (帧信息->enPixelType == PixelType_Gvsp_Mono12)
    {
        GenImage1(&ho_Image, "uint2", (帧信息->nWidth) * (ctx->分时频闪数), (帧信息->nHeight) / (ctx->分时频闪数), (__int64)图像指针);
    }
    CreateMessage(&hv_MessageHandle);
    SetMessageObj(ho_Image, hv_MessageHandle, "image");
    SetMessageTuple(hv_MessageHandle, "nFrameNum", (__int64)(帧信息->nFrameNum));
    SetMessageTuple(hv_MessageHandle, "nFrameCounter", (__int64)(帧信息->nFrameCounter));
    SetMessageTuple(hv_MessageHandle, "nTriggerIndex", (__int64)(帧信息->nTriggerIndex));
    SetMessageTuple(hv_MessageHandle, "nFirstLineEncoderCount", (__int64)(帧信息->nFirstLineEncoderCount));
    SetMessageTuple(hv_MessageHandle, "nLastLineEncoderCount", (__int64)(帧信息->nLastLineEncoderCount));
    SetMessageTuple(hv_MessageHandle, "nHostTimeStamp", (__int64)(帧信息->nHostTimeStamp));
    SetMessageTuple(hv_MessageHandle, "fGain", (DOUBLE)(帧信息->fGain));
    SetMessageTuple(hv_MessageHandle, "fExposureTime", (DOUBLE)(帧信息->fExposureTime));

    SetMessageTuple(hv_MessageHandle, "nDeviceID", ctx->相机用户名);

    EnqueueMessage((ctx->海康相机采集队列), hv_MessageHandle, HTuple(), HTuple());
    ClearMessage(hv_MessageHandle);
}

Herror 海康_打开相机(Hproc_handle proc_handle)
{

    Hcpar 相机用户名, 分时频闪数;
    Hcpar *相机采集队列容器;
    INT4_8 参数个数; // 参数个数

    HAllocStringMem(proc_handle, 64);
    HGetSPar(proc_handle, 1, STRING_PAR, &相机用户名, 1);
    HGetSPar(proc_handle, 2, LONG_PAR, &分时频闪数, 1);
    HGetPPar(proc_handle, 3, &相机采集队列容器, &参数个数);
    HTuple 相机采集队列(相机采集队列容器, 1);

    void *Hik_handle;
    int nRet;
    int i;
    int EnumDevices_num=0;
    MV_CC_DEVICE_INFO_LIST m_stDevList;
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    while (EnumDevices_num < 20)
    {
       
        nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE, &m_stDevList);
        EnumDevices_num=EnumDevices_num+1;
        // nRet = MV_CC_EnumDevices(nTLayerType, &m_stDevList);
        if (MV_OK != nRet)
        {
            return 10000 * H__LINE__;
        }
        if (m_stDevList.nDeviceNum != 0)
        {
            break;
        }
        Sleep(10);
    }
    if( m_stDevList.nDeviceNum == 0)
    {
       return 10086;
    }
    int index = -1;
    for (int i = 0; i <= m_stDevList.nDeviceNum - 1; i++)
    {
        char *a = (char *)m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chUserDefinedName;
        char *b = 相机用户名.par.s;
        int tt = strcmp(a, b);
        if (tt == 0)
        {
            index = i;
            break;
        }
    }

    if (index == -1)
    {
        return 10000 * H__LINE__;
    }
    MV_CC_DEVICE_INFO m_stDevInfo = {0};
    memcpy(&m_stDevInfo, m_stDevList.pDeviceInfo[index], sizeof(MV_CC_DEVICE_INFO));
    nRet = MV_CC_CreateHandle(&Hik_handle, &m_stDevInfo);
    // handle_data->海康相机句柄 = Hik_handle;
    nRet = MV_CC_OpenDevice(Hik_handle, MV_ACCESS_Exclusive, 0);
    if (MV_OK != nRet)
    {
        return 10000 * H__LINE__;
    }
    /* 6. 构造并填充上下文 */
    // MyContext *ctx = (MyContext *)malloc(sizeof(MyContext));
    // ctx->分时频闪数 = 分时频闪数.par.l;
    // strcpy(ctx->相机用户名, 相机用户名.par.s);
    // ctx->海康相机采集队列 = &相机采集队列;
    MyContext *ctx = new MyContext(分时频闪数.par.l, 相机用户名.par.s, 相机采集队列);

    nRet = MV_CC_RegisterImageCallBackEx(Hik_handle, 海康_回调取图函数, ctx);
    if (MV_OK != nRet)
    {
        return 10000 * H__LINE__;
    }

    HUserHandleData **handle_data;
    // 分配输出句柄
    HCkP(HAllocOutputHandle(proc_handle, 1, &handle_data, &HandleTypeUser));
    // 分配并初始化用户数据
    HCkP(HAlloc(proc_handle, sizeof(HUserHandleData), (void **)handle_data));
    (*handle_data)->海康相机句柄 = Hik_handle;

    return 正确;
}
Herror 海康_开始拍摄(Hproc_handle proc_handle)
{
    HUserHandleData *handle_data;
    HGetCElemH1(proc_handle, 1, &HandleTypeUser, &handle_data);
    int 函数返回值 = MV_CC_StartGrabbing(handle_data->海康相机句柄);
    if (MV_OK == 函数返回值)
    {
        return 正确;
    }
    else
    {
        return 函数返回值;
    }
}
Herror 海康_停止拍摄(Hproc_handle proc_handle)
{
    HUserHandleData *handle_data;
    HGetCElemH1(proc_handle, 1, &HandleTypeUser, &handle_data);

    int 函数返回值 = MV_CC_StopGrabbing(handle_data->海康相机句柄);
    if (MV_OK == 函数返回值)
    {
        return 正确;
    }
    else
    {
        return 函数返回值;
    }
}
Herror 海康_关闭相机(Hproc_handle proc_handle)
{
    HUserHandleData *handle_data;
    HGetCElemH1(proc_handle, 1, &HandleTypeUser, &handle_data);

    int 函数返回值 = MV_CC_CloseDevice(handle_data->海康相机句柄);
    if (MV_OK == 函数返回值)
    {
        return 正确;
    }
    else
    {
        return 函数返回值;
    }
}
Herror 海康_设置浮点数类型参数(Hproc_handle proc_handle)
{
    HUserHandleData *handle_data;
    Hcpar 参数名称, 参数值;
    HAllocStringMem(proc_handle, 64);
    HGetCElemH1(proc_handle, 1, &HandleTypeUser, &handle_data);

    HGetSPar(proc_handle, 2, STRING_PAR, &参数名称, 1);
    HGetSPar(proc_handle, 3, DOUBLE_PAR, &参数值, 1);

    int 函数返回值 = MV_CC_SetFloatValue(handle_data->海康相机句柄, 参数名称.par.s, 参数值.par.d);
    if (MV_OK == 函数返回值)
    {
        return 正确;
    }
    else
    {
        return 函数返回值;
    }
}
Herror 海康_设置命令类型参数(Hproc_handle proc_handle)
{
    HUserHandleData *handle_data;
    Hcpar 参数名称;
    HAllocStringMem(proc_handle, 64);
    HGetCElemH1(proc_handle, 1, &HandleTypeUser, &handle_data);

    HGetSPar(proc_handle, 2, STRING_PAR, &参数名称, 1);

    return MV_CC_SetCommandValue(handle_data->海康相机句柄, 参数名称.par.s);
}
Herror 海康_设置枚举数类型参数(Hproc_handle proc_handle)
{
    HUserHandleData *handle_data;

    Hcpar 参数名称, 参数值;
    HAllocStringMem(proc_handle, 64);
    HGetCElemH1(proc_handle, 1, &HandleTypeUser, &handle_data);

    HGetSPar(proc_handle, 2, STRING_PAR, &参数名称, 1);
    HGetSPar(proc_handle, 3, LONG_PAR, &参数值, 1);

    int 函数返回值 = MV_CC_SetEnumValue(handle_data->海康相机句柄, 参数名称.par.s, 参数值.par.l);
    if (MV_OK == 函数返回值)
    {
        return 正确;
    }
    else
    {
        return 函数返回值;
    }
}
Herror 海康_设置布尔数类型参数(Hproc_handle proc_handle)
{
    HUserHandleData *handle_data;

    Hcpar 参数名称, 参数值;
    HAllocStringMem(proc_handle, 64);
    HGetCElemH1(proc_handle, 1, &HandleTypeUser, &handle_data);

    HGetSPar(proc_handle, 2, STRING_PAR, &参数名称, 1);
    HGetSPar(proc_handle, 3, LONG_PAR, &参数值, 1);

    if (参数值.par.l == 0)
    {
        int 函数返回值 = MV_CC_SetBoolValue(handle_data->海康相机句柄, 参数名称.par.s, 0);
        if (MV_OK == 函数返回值)
        {
            return 正确;
        }
        else
        {
            return 函数返回值;
        }
    }
    else if (参数值.par.l == 1)
    {
        int 函数返回值 = MV_CC_SetBoolValue(handle_data->海康相机句柄, 参数名称.par.s, 1);
        if (MV_OK == 函数返回值)
        {
            return 正确;
        }
        else
        {
            return 函数返回值;
        }
    }
    else
    {
        return __LINE__;
    }
}
Herror 海康_设置整数类型参数(Hproc_handle proc_handle)
{
    HUserHandleData *handle_data;

    Hcpar 参数名称, 参数值;
    HAllocStringMem(proc_handle, 64);
    HGetCElemH1(proc_handle, 1, &HandleTypeUser, &handle_data);

    HGetSPar(proc_handle, 2, STRING_PAR, &参数名称, 1);
    HGetSPar(proc_handle, 3, LONG_PAR, &参数值, 1);
    int 函数返回值 = MV_CC_SetIntValueEx(handle_data->海康相机句柄, 参数名称.par.s, 参数值.par.l);
    if (MV_OK == 函数返回值)
    {
        return 正确;
    }
    else
    {
        return 函数返回值;
    }
}
Herror 海康_打开采集卡(Hproc_handle proc_handle)
{

    Hcpar 设备序号;
    Hcpar 采集卡序列号;
    HAllocStringMem(proc_handle, 64);
    HGetSPar(proc_handle, 1, LONG_PAR, &设备序号, 1);
    HGetSPar(proc_handle, 2, STRING_PAR, &采集卡序列号, 1);

    // int 函数返回值 = MV_OK;
    // // ch:初始化SDK | en:Initializeint SDK
    // // MV_CC_Initialize();
    // // ch: 采集卡接口类型 | en: The interface type of frame grabber
    // int nInterfaceType = MV_GIGE_INTERFACE | MV_CAMERALINK_INTERFACE | MV_CXP_INTERFACE | MV_XOF_INTERFACE | MV_VIR_INTERFACE;

    // {
    //     // ch:初始化SDK | en:Initialize SDK
    //     函数返回值 = MV_CC_Initialize();
    //     if (MV_OK != 函数返回值)
    //     {
    //         // printf("Initialize SDK fail! nRet [0x%x]\n", nRet);
    //         return 10000 + 函数返回值;
    //     }
    //     MV_CC_CreateInterfaceByID(&海康采集卡句柄[设备序号.par.l], 采集卡序列号.par.s);
    //     // ch: 打开采集卡 | en: Open the frame grabber
    //     函数返回值 = MV_CC_OpenInterface(海康采集卡句柄[设备序号.par.l], NULL);
    //     if (MV_OK == 函数返回值)
    //     {
    //     }
    //     else
    //     {
    //         return 10000 + 函数返回值;
    //     }
    return 正确;
    //}
}
