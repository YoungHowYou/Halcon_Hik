#include "Halcon.h"
#define EXPORTS_API __declspec(dllexport)
#define 正确 2
#ifdef __cplusplus
extern "C" {
#endif

#pragma region HikCamera
EXPORTS_API Herror 海康_打开采集卡(Hproc_handle proc_handle);
EXPORTS_API Herror 海康_打开相机(Hproc_handle proc_handle);
EXPORTS_API Herror 海康_开始拍摄(Hproc_handle proc_handle);
EXPORTS_API Herror 海康_停止拍摄(Hproc_handle proc_handle);
EXPORTS_API Herror 海康_关闭相机(Hproc_handle proc_handle);
EXPORTS_API Herror 海康_设置浮点数类型参数(Hproc_handle proc_handle);
EXPORTS_API Herror 海康_设置命令类型参数(Hproc_handle proc_handle);
EXPORTS_API Herror 海康_设置枚举数类型参数(Hproc_handle proc_handle);
EXPORTS_API Herror 海康_设置布尔数类型参数(Hproc_handle proc_handle);
EXPORTS_API Herror 海康_设置整数类型参数(Hproc_handle proc_handle);
EXPORTS_API Herror 海康_获取浮点数类型参数(Hproc_handle proc_handle);
EXPORTS_API Herror 海康_获取枚举数类型参数(Hproc_handle proc_handle);
EXPORTS_API Herror 海康_获取布尔数类型参数(Hproc_handle proc_handle);
EXPORTS_API Herror 海康_获取整数类型参数(Hproc_handle proc_handle);
EXPORTS_API Herror 海康_获取字符串类型参数(Hproc_handle proc_handle);
#pragma endregion



#ifdef __cplusplus
}
#endif 


