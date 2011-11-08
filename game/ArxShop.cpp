// 8th Nov 2011 - Solarsplace - Arx End Of Sun

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
#include "ArxShop.h"

/************************************************************************/
/************************************************************************/
/************************************************************************/

/*** !!! IMPLEMENT SAVE / LOAD !!!

/************************************************************************/
/************************************************************************/
/************************************************************************/

idArxShop::idArxShop() {

	shopSlotItem_Icon = new idDict();

}

idArxShop::~idArxShop() {
}

const int ARX_MAX_SHOP_SLOTS = 48; // 0 to 47

const idStr ARX_REC_SEP = "<@@@ARX@@@>";
const idStr ARX_PROP_SHOP = "ARX_ENT_SHOP";
const idStr ARX_PROP_SHOP_ITEM = "ARX_ENT_SHOP_ITEM_";

void idArxShop::LoadActiveShop( idEntity *shopEntity )
{
	//REMOVEME
	gameLocal.Printf( "Shop LoadActiveShop = %s\n", shopEntity->name.c_str() );

	idStr iconName;
	const idDeclEntityDef *shopItemDef = NULL;

	int itemCount = 0;

	idStr shopItemClass;
	
	idStr currentMapName = gameLocal.GetMapName();
	idStr currentShopName = shopEntity->name;
	idStr currentShopIDString = currentMapName + ARX_REC_SEP + ARX_PROP_SHOP + ARX_REC_SEP + currentShopName;
	idStr currentShopSlotItem;

	/******************************************************************************/
	/******************************************************************************/

	if ( !gameLocal.persistentLevelInfo.GetInt( currentShopIDString ) ) // If this is the first time the payer has 'been' to this shop then store its contents in the persistent dictionary
	{
		itemCount = 0;

		for (int i = 0; i < ARX_MAX_SHOP_SLOTS; i++)
		{
			shopItemClass = shopEntity->spawnArgs.GetString( va( "shopItem_%i", i ), "" ); // Get the class name for this shop item

			if ( idStr::Icmp( "", shopItemClass ) ) // Returns 0 if the text is equal
			{ 
				// Save the item in the persistent dictionary
				currentShopSlotItem = currentMapName + ARX_REC_SEP + ARX_PROP_SHOP + ARX_REC_SEP + currentShopName + ARX_REC_SEP + va( ARX_PROP_SHOP_ITEM + "%i", i );

				gameLocal.persistentLevelInfo.Set( currentShopSlotItem, shopItemClass );

				itemCount ++;
			}
		}

		// Store the number of items in this shop - also used as a flag to see if we need to store this shop.
		gameLocal.persistentLevelInfo.SetInt ( currentShopIDString, itemCount );
	}

	
	/******************************************************************************/
	/******************************************************************************/

	// Load the shop into the dictionaries
	shopSlotItem_Icon->Clear();
	shopSlotItem_Class->Clear();

	for (int i = 0; i < ARX_MAX_SHOP_SLOTS; i++)
	{
		currentShopSlotItem = currentMapName + ARX_REC_SEP + ARX_PROP_SHOP + ARX_REC_SEP + currentShopName + ARX_REC_SEP + va( ARX_PROP_SHOP_ITEM + "%i", i );

		shopItemDef = gameLocal.FindEntityDef( gameLocal.persistentLevelInfo.GetString( currentShopSlotItem ), false );

		if ( shopItemDef )
		{

			gameLocal.Printf( "wank = %s\n", currentShopSlotItem.c_str() );

			shopItemDef->dict.GetString( "inv_icon", "", iconName );


			//REMOVEME
			gameLocal.Printf( "Shop inv_icon = %s\n", iconName.c_str() );
		}

	}

	itemCount = gameLocal.persistentLevelInfo.GetInt( currentShopIDString );

	//shopSlotItem_Icon
				// Get the .def for this shop item
				//shopItemDef = gameLocal.FindEntityDef( "shopItemClass", false );

}