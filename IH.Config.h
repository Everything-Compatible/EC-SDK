#pragma once

struct ConfData
{
	bool UseOriginalCCFile{ false };

	int Reserved[31];
};

static_assert(sizeof(ConfData) == 128);

ConfData& GetConfigData();
void UseOriginalFileClass(bool Option);
bool UseOriginalFileClass();

struct UseOriginalFileClass_Scoped
{
private:
	bool Previous;
public:
	UseOriginalFileClass_Scoped()
	{
		Previous = UseOriginalFileClass();
		UseOriginalFileClass(true);
	}

	UseOriginalFileClass_Scoped(bool bUse)
	{
		Previous = UseOriginalFileClass();
		UseOriginalFileClass(bUse);
	}

	~UseOriginalFileClass_Scoped()
	{
		UseOriginalFileClass(Previous);
	}
};