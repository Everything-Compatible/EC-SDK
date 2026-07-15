#pragma once
#include "IH.Loader.h"

namespace InitialLoad
{
	template<typename ParamType, typename... TArgs>
	IHInitialLoadService CreateRequest(const char* Name, TArgs&&... args)//创建后永不销毁
	{
		static_assert(std::is_base_of<InitialLoadParam, ParamType>::value, "ParamType 必须派生自 InitialLoadParam ！");
		return IHInitialLoadService{ Name,new ParamType(std::forward<TArgs>(args)...) };
	}
	void ServiceRequest(IHInitialLoadService IService);
	template<typename ParamType, typename... TArgs>
	void CreateRequestAndSubmit(const char* Name, TArgs&&... args)//创建后永不销毁
	{
		static_assert(std::is_base_of<InitialLoadParam, ParamType>::value, "ParamType 必须派生自 InitialLoadParam ！");
		ServiceRequest(IHInitialLoadService{ Name,new ParamType(std::forward<TArgs>(args)...) });
	}
	template<typename ParamType>
	class Service
	{
		static_assert(std::is_base_of<InitialLoadParam, ParamType>::value, "ParamType 必须派生自 InitialLoadParam ！");
		std::string ServiceName;
		PArray<IHInitialLoadService> Requests;
		bool Requested{ false };
	public:
		Service() = delete;
		Service(const Service&) = delete;
		Service(Service&&) = delete;

		Service(const std::string& Name) : ServiceName(Name), Requests() {}
		Service(const char* Name) : ServiceName(Name), Requests() {}
		void ProcessOnce(const std::function<void(const ParamType&)>& Func)//如果列表为空则填补并处理
		{
			if (!Requested)
			{
				Refresh();
				for (size_t i = 0; i < Requests.N; i++)
					if (Requests.Data[i].Param)
						Func(*static_cast<const ParamType* const>(Requests.Data[i].Param));
				Requested = true;
			}
		}
		void Process(const std::function<void(const ParamType&)>& Func)//如果列表为空仍会尝试refresh
		{
			if (!Requests.N)Refresh();
			for (size_t i = 0; i < Requests.N; i++)
				if (Requests.Data[i].Param)
					Func(*static_cast<const ParamType* const>(Requests.Data[i].Param));
		}
		void Refresh()
		{
			Requests = Init::QueryServiceRequest(ServiceName.c_str());
		}
		void RefreshAndProcess(const std::function<void(const ParamType&)>& Func)
		{
			Refresh();
			Process(Func);
		}
	};
}

#define InitialRequest(Type, ...) InitialLoad::CreateRequestAndSubmit<InitialLoadParam_##Type>(__VA_ARGS__)

#define ClassRegisterByName(Type, Class) InitialLoad::CreateRequestAndSubmit<InitialLoadParam_RegisterVTable>(Type, #Class, GetIHFileRegisterKey<Class>(), sizeof(Class));

#define RegisterIHFile(Class) ClassRegisterByName("IHFile::RegisterIHFile", Class)

#define RegisterIHFileFilter(Class, Filter) InitialLoad::CreateRequestAndSubmit<InitialLoadParam_RegisterFunction>("IHFile::RegisterIHFileFilter", #Class, Filter);

#define RegisterIHFileBinding(Class, FileName) InitialLoad::CreateRequestAndSubmit<InitialLoadParam_RedirectFile>("IHFile::BindToStream", FileName, #Class);

#define RegisterIHFileTag(Class, TagName, Value) InitialLoad::CreateRequestAndSubmit<InitialLoadParam_RegisterTag>("IHFile::RegisterIHFileTag", #Class, TagName, Value);
//这个标签仅限只读文件类型，限制启动缓存访问的尺寸下限，默认值为1MB，如果在打了缓存访问标签的文件上写入数据则标签失效
#define RegisterIHFileCachedAccessThreshold(Class, Threshold) RegisterIHFileTag(Class, "IHFile::CachedAccessThreshold", Threshold)
//这个标签仅限只读文件类型，表示文件的访问迭代类型，会针对不适合随机访问的文件类型进行缓存访问优化
#define RegisterIHFileIterationType(Class, IterType) RegisterIHFileTag(Class, "IHFile::IterationType", static_cast<int>(IterType));

#define RegisterAddressCommentProvider(Name, Provider) InitialLoad::CreateRequestAndSubmit<InitialLoadParam_RegisterFunction>("EC::RegisterAddressCommentProvider", Name, Provider);