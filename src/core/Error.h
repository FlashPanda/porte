
#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CORE_ERROR_H
#define PORTE_CORE_ERROR_H

#include <core/porte.h>

namespace porte {

// 错误报告的声明

// 设置printf格式
#define PRINTF_FUNC
void Warning(const char *, ...) PRINTF_FUNC;
void Error(const char *, ...) PRINTF_FUNC;

}  // namespace porte

#endif  // PORTE_CORE_ERROR_H
