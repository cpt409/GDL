
#pragma once

#include "Container.h"

class CChestWeenie : public CContainerWeenie
{
public:
	CChestWeenie();
	virtual ~CChestWeenie() override;

	virtual class CChestWeenie *AsChest() { return this; }

	virtual void ApplyQualityOverrides() override;

	virtual void PostSpawn() override;

	virtual void OnContainerOpened(CWeenieObject *other) override;
	virtual void OnContainerClosed(CWeenieObject *other) override;
};
