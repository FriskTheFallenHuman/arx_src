// 23rd April 2010 - Solarsplace - Arx End Of Sun

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
#include "ArxShop.h"

idArxShop::idArxShop() {

}

idArxShop::~idArxShop() {
}

const int ARX_MAX_SHOP_SLOTS = 48; // 0 to 47

const idStr ARX_REC_SEP = "<@@@ARX@@@>";
const idStr ARX_PROP_SHOP = "ARX_ENT_SHOP";
const idStr ARX_PROP_SHOP_ITEM_ = "ARX_ENT_SHOP_ITEM_";

void idArxShop::LoadActiveShop( idEntity shopEntity )
{

	idStr shopItemClass;
	const idDeclEntityDef *shopItemDef = NULL;

	idStr currentMapName = gameLocal.GetMapName();
	idStr currentShopName = shopEntity.name;

	idStr currentShopIDString = currentMapName + ARX_REC_SEP + ARX_PROP_SHOP + ARX_REC_SEP + currentShopName;


	// If this is the first time the payer has 'been' to this shop then store its
	// contents in the persistent dictionary
	if ( !gameLocal.persistentLevelInfo.GetBool( currentShopIDString ) )
	{
		// Just a token to test whether we have this shop saved
		gameLocal.persistentLevelInfo.SetBool ( currentShopIDString, "1" );

		for (int i = 0; i < ARX_MAX_SHOP_SLOTS; i++)
		{
			// Get the class name for this shop item
			shopItemClass = shopEntity.spawnArgs.GetString( va( "shopItem_%i", i ), "" );

			// SP - Returns 0 if the text is equal
			if ( idStr::Icmp( "", shopItemClass ) )
			{ 
				// Save the item in the persistent dictionary


				// Get the .def for this shop item
				//shopItemDef = gameLocal.FindEntityDef( "shopItemClass", false );

			}
		}



	}

}