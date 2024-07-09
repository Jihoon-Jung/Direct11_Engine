#pragma once

class ShaderSlot
{
public:
	ShaderSlot() {}
	~ShaderSlot() {};

	void SetSlot(wstring name, int slot) {
		for (int i = 0; i < slots.size(); i++)
		{
			auto it = slots[i].find(name);
			if (it != slots[i].end())
			{
				printf("That slot is already set up");
				return;
			}
		}
		map<wstring, int> map;
		map[name] = slot;
		slots.push_back(map);
	}
	int GetSlotNumber(wstring name)
	{
		for (int i = 0; i < slots.size(); i++)
		{
			auto it = slots[i].find(name);
			if (it != slots[i].end())
			{
				return it->second;
			}
		}
	}

private:
	vector<map<wstring, int>> slots;
};

