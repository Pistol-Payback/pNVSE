namespace CompareWeaponStats
{
	void __cdecl InventoryRewriteWeaponStatsSingleProjectile(char* dst, size_t len, char* format, int damage)
	{
		sprintf(dst, format, damage);

		int equippedDamage; //= round(GetPlayerWeaponDamage());

		if (equippedDamage && equippedDamage != damage)
		{
			strcat(dst, damage > equippedDamage ? "+" : "-");
		}
	}

	void __cdecl InventoryRewriteWeaponStatsMultiProjectile(char* dst, size_t len, char* format, double damagePerProj, int numProjectiles)
	{
		sprintf(dst, format, damagePerProj, numProjectiles);

		int damage = round(damagePerProj * numProjectiles);
		int equippedDamage; //= round(GetPlayerWeaponDamage());

		if (equippedDamage && (equippedDamage != damage))
		{
			strcat(dst, damage > equippedDamage ? "+" : "-");
		}
	}

	void __cdecl InventoryRewriteWeaponStatsDPS(char* dst, size_t len, char* format, int dps)
	{
		sprintf(dst, format, dps);

		int equippedDPS; // = round(GetPlayerWeaponDPS());
		if (equippedDPS && equippedDPS != dps)
		{
			strcat(dst, dps > equippedDPS ? "+" : "-");
		}
	}

	void InitHooks()
	{
		//WriteRelCall(0x7083FA, UInt32(InventoryCompareWeaponStatsSingleProjectile));
		//WriteRelCall(0x7083C1, UInt32(InventoryCompareWeaponStatsMultiProjectile));
		//WriteRelCall(0x708324, UInt32(InventoryCompareWeaponStatsDPS));
	}
}