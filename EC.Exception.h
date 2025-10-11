#pragma once
#include <exception>
#include <stdexcept>

/*
ECDispatchException ：调用扩展接口时可能会抛出的异常。
与DispatchInterface等接口派发类有关。
*/
class ECDispatchException :public std::exception
{
	const char* Info;
public:
	ECDispatchException(const char* info);
	const char* what() const noexcept override;
};

/*
 SIException ：调用WIC接口时可能会抛出的异常。
 可能抛出SIException的情况有两种：
 一、WIC未成功载入。请在调用WIC的API之前通过SI::Available()检查。
	在返回一次true之后，WIC就总是可以正确加载的了。
	由于载入发生在对局开始之前，通常只需开局时检查一次。
 二、WIC载入了，但无法获取API。
	很可能是包含了新版的SDK而实际装载了旧的DLL。
	通过更新WIC的版本可以基本解决问题。
 你可以在调用API的时候放进try块当中，然后catch相关问题，
 或确保抛出异常的情况不会发生。
*/
class SIException :public ECDispatchException
{
public:
	SIException(const char* info);
	const char* what() const noexcept override;
};
