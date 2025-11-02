
#include <stdio.h>
#include "Halcon_Hik.h"

 Herror cHik_openCaptureCard(Hproc_handle proc_handle)
 {
  return 海康_打开采集卡( proc_handle);
 }
 Herror cHik_openCamera(Hproc_handle proc_handle)
 {
  return 海康_打开相机( proc_handle);
 }
 Herror cHik_StartPhotograph(Hproc_handle proc_handle)
 {
  return 海康_开始拍摄( proc_handle);
 }
 Herror cHik_StopPhotograph(Hproc_handle proc_handle)
 {
  return 海康_停止拍摄( proc_handle);
 }
 Herror cHik_closeCamera(Hproc_handle proc_handle)
 {
  return 海康_关闭相机( proc_handle);
 }
 Herror cHik_SetfloatParameters(Hproc_handle proc_handle)
 {
  return 海康_设置浮点数类型参数( proc_handle);
 }
 Herror cHik_SetCommandParameters(Hproc_handle proc_handle)
 {
  return 海康_设置命令类型参数( proc_handle);
 }
 Herror cHik_SetEnumsParameters(Hproc_handle proc_handle)
 {
  return 海康_设置枚举数类型参数( proc_handle);
 }
 Herror cHik_SetBoolParameters(Hproc_handle proc_handle)
 {
  return 海康_设置布尔数类型参数( proc_handle);
 }
 Herror cHik_SetIntParameters(Hproc_handle proc_handle)
 {
  return 海康_设置整数类型参数( proc_handle);
 }