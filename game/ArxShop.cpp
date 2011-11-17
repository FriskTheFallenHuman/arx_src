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

	shopSlotItem_Class = new idDict();

	totalUsedShopSlots = 0;

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

	const char *result;
	idStr shop_item_icon;
    idStr shop_item_name;
    idStr shop_item_value;
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
	shopSlotItem_Class->Clear();
	itemCount = 0;

	for (int i = 0; i < ARX_MAX_SHOP_SLOTS; i++)
	{
		// Load this shops items 1 by 1 from the persitent dictionary
		currentShopSlotItem = currentMapName + ARX_REC_SEP + ARX_PROP_SHOP + ARX_REC_SEP + currentShopName + ARX_REC_SEP + va( ARX_PROP_SHOP_ITEM + "%i", i );
		gameLocal.persistentLevelInfo.GetString( currentShopSlotItem, "", &result );

		shopItemDef = gameLocal.FindEntityDef( result, false );

		if ( shopItemDef )
		{
			//REMOVEME
			gameLocal.Printf( "Dict key = %s\n", currentShopSlotItem.c_str() );
			
			// Load data for shop from item defs
			shopItemDef->dict.GetString( "inv_icon", "guis/assets/icons/404_icon.tga", shop_item_icon ); // Hopefully people with 'get' that 404 image = inv_icon is not set for this item! 
			shopItemDef->dict.GetString( "inv_name", "name = 404", shop_item_name ); // Hopefully people with 'get' that name = 404  = inv_name is not set for this item! 
			shopItemDef->dict.GetString( "shop_item_value", "404", shop_item_value ); // Hopefully people with 'get' that 404 gold coins = shop_item_value is not set for this item! 

			// Grouping of identical items or not.
			int existingShopItemIndex = FindShopItem( result );

			//REMOVEME
			gameLocal.Printf( "existingShopItemIndex( %s ) = %i\n", result , existingShopItemIndex );

			if ( shopItemDef->dict.GetBool( "inventory_nostack", "0" ) || existingShopItemIndex == -1 )
			{
				// Store the non-grouped / non-stacked shop data in the shop dictionary || store a new single groupable / stackable item for the first time.
				shopSlotItem_Class->Set( va( "shop_item_class_%i", itemCount ), result );
				shopSlotItem_Class->Set( va( "shop_item_icon_%i", itemCount ), shop_item_icon.c_str() );
				shopSlotItem_Class->Set( va( "shop_item_name_%i", itemCount ), shop_item_name.c_str() );
				shopSlotItem_Class->Set( va( "shop_item_value_%i", itemCount ), shop_item_value.c_str() );
				shopSlotItem_Class->Set( va( "shop_item_count_%i", itemCount ), "1" );

				itemCount ++;
			}
			else
			{
				// We already have one of these in the shop and we can stack / group it.
				int itemGroupCount = shopSlotItem_Class->GetInt( va( "shop_item_count_%i", existingShopItemIndex ), "0" ) + 1;
				shopSlotItem_Class->SetInt( va( "shop_item_count_%i", existingShopItemIndex ), itemGroupCount );
			}

			//REMOVEME
			gameLocal.Printf( "shop_item_icon_%i = %s\n", itemCount, shop_item_icon.c_str() );
			gameLocal.Printf( "shop_item_class_%i = %s\n", itemCount, result );	
		}

	}

	totalUsedShopSlots = itemCount;

	//itemCount = gameLocal.persistentLevelInfo.GetInt( currentShopIDString );

}

void idArxShop::RemoveShopItem( int slotId )
{
	idDict tempShopSlotItem_Class;
	int itemGroupCount;

	itemGroupCount = shopSlotItem_Class->GetInt( va( "shop_item_count_%i", slotId ), "0" );

	if ( itemGroupCount > 1 ) { // More than 1 of these grouped items left. Just reduce the count available.
		
		shopSlotItem_Class->SetInt( va( "shop_item_count_%i", slotId ), itemGroupCount - 1 );
	}
	else { // Only 1 item in slot or 1 grouped item left. Remove from shop.

		// Copy the shop into a temp dictionary.
		shopSlotItem_Class->Copy( tempShopSlotItem_Class );

		// Clear the shop dictionary.
		shopSlotItem_Class->Clear();

		// Copy back the items 1 by 1 ommitting the empty slots.
		for (int i = 0; i < ARX_MAX_SHOP_SLOTS; i++) {

			if ( tempShopSlotItem_Class.GetInt( va( "shop_item_count_%i", i ), "0" ) > 0 ) {

				shopSlotItem_Class->Set( va( "shop_item_class_%i", i ), tempShopSlotItem_Class.GetString( "shop_item_class_%i" ) );
				shopSlotItem_Class->Set( va( "shop_item_icon_%i", i ), tempShopSlotItem_Class.GetString( "shop_item_icon_%i" ) );
				shopSlotItem_Class->Set( va( "shop_item_name_%i", i ), tempShopSlotItem_Class.GetString( "shop_item_name_%i" ) );
				shopSlotItem_Class->Set( va( "shop_item_value_%i", i ), tempShopSlotItem_Class.GetString( "shop_item_value_%i" ) );
				shopSlotItem_Class->Set( va( "shop_item_count_%i", i ), tempShopSlotItem_Class.GetString( "shop_item_count_%i" ) );
			}
		}
	}
}

int idArxShop::FindShopItem( const char *name ) {

	const char *shop_item;

	for ( int i = 0; i < ARX_MAX_SHOP_SLOTS; i++ ) {

		shop_item = shopSlotItem_Class->GetString( va( "shop_item_class_%i", i ) );
		 
		if ( shop_item && *shop_item ) {

			//REMOVEME
			gameLocal.Printf( "(%i): name = %s and shop_item = %s\n", i, name, shop_item );

			if ( idStr::Icmp( name, shop_item ) == 0 ) {

				//REMOVEME
				gameLocal.Printf( "FindShopItem returns: %i\n", i );

				return i;
			}
		}
	}

	//REMOVEME
	gameLocal.Printf( "FindShopItem returns: -1\n" );

	return -1;
}