// 8th Nov 2011 - Solarsplace - Arx End Of Sun

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

//CLASS_DECLARATION(idClass, idArxShop)
//END_CLASS

/************************************************************************/
/************************************************************************/
/************************************************************************/

/*** !!! IMPLEMENT SAVE / LOAD !!!

Some info is here: http://www.doom3world.org/phpbb2/viewtopic.php?f=56&t=9490

/************************************************************************/
/************************************************************************/
/************************************************************************/

idArxShop::idArxShop() {

	shopSlotItem_Dict = new idDict();

	totalUsedShopSlots = 0;
}

idArxShop::~idArxShop() {
}

const int ARX_MAX_SHOP_SLOTS = 48; // 0 to 47

const idStr ARX_REC_SEP = "<@@@ARX@@@>";
const idStr ARX_PROP_SHOP = "ARX_ENT_SHOP";
const idStr ARX_PROP_SHOP_ITEM = "ARX_ENT_SHOP_ITEM_";

/* NOT implemented properly yet!
void idArxShop::Save( idSaveGame *savefile ) const {

	//REMOVEME
	gameLocal.Printf( "idArxShop::Save\n" );

	savefile->WriteInt( totalUsedShopSlots );
	savefile->WriteDict( shopSlotItem_Dict );
}

void idArxShop::Restore( idRestoreGame *savefile ) {

		//REMOVEME
	gameLocal.Printf( "idArxShop::Restore\n" );

	savefile->ReadInt( totalUsedShopSlots );
	savefile->ReadDict( shopSlotItem_Dict );
}
*/

void idArxShop::LoadActiveShop( idEntity *shopEntity )
{
	//REMOVEME
	gameLocal.Printf( "Shop LoadActiveShop = %s\n", shopEntity->name.c_str() );

	const char *result;
	idStr shop_item_icon;
    idStr shop_item_name;
    idStr inv_shop_item_value;
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
		gameLocal.Printf( "LoadActiveShop new shop = %s\n", currentShopIDString.c_str() );

		itemCount = 0;

		for (int i = 0; i < ARX_MAX_SHOP_SLOTS; i++)
		{
			shopItemClass = shopEntity->spawnArgs.GetString( va( "shopItem_%i", i ), "" ); // Get the class name for this shop item

			gameLocal.Printf( "shopItem_%i = shopItemClass %s\n", i, shopItemClass.c_str() );

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
	shopSlotItem_Dict->Clear();
	itemCount = 0;

	for (int i = 0; i < ARX_MAX_SHOP_SLOTS; i++)
	{
		// Load this shops items 1 by 1 from the persitent dictionary
		currentShopSlotItem = currentMapName + ARX_REC_SEP + ARX_PROP_SHOP + ARX_REC_SEP + currentShopName + ARX_REC_SEP + va( ARX_PROP_SHOP_ITEM + "%i", i );
		gameLocal.persistentLevelInfo.GetString( currentShopSlotItem, "", &result );

		if ( idStr::Icmp( "", result ) == 0 )
		{ continue; }

		shopItemDef = gameLocal.FindEntityDef( result, false );

		if ( shopItemDef )
		{
			//REMOVEME
			gameLocal.Printf( "Dict key = %s\n", currentShopSlotItem.c_str() );
			
			// Load data for shop from item defs
			shopItemDef->dict.GetString( "inv_icon", "guis/assets/icons/404_icon.tga", shop_item_icon ); // Hopefully people with 'get' that 404 image = inv_icon is not set for this item! 
			shopItemDef->dict.GetString( "inv_name", "name = 404", shop_item_name ); // Hopefully people with 'get' that name = 404  = inv_name is not set for this item! 
			shopItemDef->dict.GetString( "inv_shop_item_value", "404", inv_shop_item_value ); // Hopefully people with 'get' that 404 gold coins = inv_shop_item_value is not set for this item! 

			// Grouping of identical items or not.
			int existingShopItemIndex = FindShopItem( result );

			if ( shopItemDef->dict.GetBool( "inventory_nostack", "0" ) || existingShopItemIndex == -1 )
			{
				// Store the non-grouped / non-stacked shop data in the shop dictionary || store a new single groupable / stackable item for the first time.
				shopSlotItem_Dict->Set( va( "shop_item_class_%i", itemCount ), result );
				shopSlotItem_Dict->Set( va( "shop_item_icon_%i", itemCount ), shop_item_icon.c_str() );
				shopSlotItem_Dict->Set( va( "shop_item_name_%i", itemCount ), shop_item_name.c_str() );
				shopSlotItem_Dict->Set( va( "inv_shop_item_value_%i", itemCount ), inv_shop_item_value.c_str() );
				shopSlotItem_Dict->Set( va( "shop_item_count_%i", itemCount ), "1" );

				itemCount ++;
			}
			else
			{
				// We already have one of these in the shop and we can stack / group it.
				int itemGroupCount = shopSlotItem_Dict->GetInt( va( "shop_item_count_%i", existingShopItemIndex ), "0" ) + 1;
				shopSlotItem_Dict->SetInt( va( "shop_item_count_%i", existingShopItemIndex ), itemGroupCount );
			}

		}
		else
		{
			gameLocal.Printf( "ERROR in shop '%s'. The entity def for '%s' could not be found.\n", currentShopName.c_str(), result );
		}

	}

	totalUsedShopSlots = itemCount;

	//itemCount = gameLocal.persistentLevelInfo.GetInt( currentShopIDString );

}

void idArxShop::AddShopItem( const char *className )
{

	int existingShopItemIndex = -1;
	idStr shop_item_icon;
    idStr shop_item_name;
    idStr inv_shop_item_value;
	const idDeclEntityDef *shopItemDef = NULL;

	// Get the .def for the item the shop is buying
	shopItemDef = gameLocal.FindEntityDef( className, false );
	
	if ( shopItemDef ) {

		// Load data for shop from item defs
		shopItemDef->dict.GetString( "inv_icon", "guis/assets/icons/404_icon.tga", shop_item_icon ); // Hopefully people with 'get' that 404 image = inv_icon is not set for this item! 
		shopItemDef->dict.GetString( "inv_name", "name = 404", shop_item_name ); // Hopefully people with 'get' that name = 404  = inv_name is not set for this item! 
		shopItemDef->dict.GetString( "inv_shop_item_value", "404", inv_shop_item_value ); // Hopefully people with 'get' that 404 gold coins = inv_shop_item_value is not set for this item! 

		// Do we have one of these items already?
		existingShopItemIndex = FindShopItem( className );

		//REMOVEME
		gameLocal.Printf( "AddShopItem %i = %s\n", totalUsedShopSlots, className );	

		if ( shopItemDef->dict.GetBool( "inventory_nostack", "0" ) || existingShopItemIndex == -1 )
		{
			// Store the non-grouped / non-stacked shop data in the shop dictionary || store a new single groupable / stackable item for the first time.
			shopSlotItem_Dict->Set( va( "shop_item_class_%i", totalUsedShopSlots ), className );
			shopSlotItem_Dict->Set( va( "shop_item_icon_%i", totalUsedShopSlots ), shop_item_icon.c_str() );
			shopSlotItem_Dict->Set( va( "shop_item_name_%i", totalUsedShopSlots ), shop_item_name.c_str() );
			shopSlotItem_Dict->Set( va( "inv_shop_item_value_%i", totalUsedShopSlots ), inv_shop_item_value.c_str() );
			shopSlotItem_Dict->Set( va( "shop_item_count_%i", totalUsedShopSlots ), "1" );

			// Add a new item
			totalUsedShopSlots ++;
		} else {
			// Item already exists in the shop - increment its group count
			int itemGroupCount = shopSlotItem_Dict->GetInt( va( "shop_item_count_%i", existingShopItemIndex ), "0" ) + 1;
			shopSlotItem_Dict->SetInt( va( "shop_item_count_%i", existingShopItemIndex ), itemGroupCount );
		}

	}
}

void idArxShop::RemoveShopItem( int slotId )
{
	//REMOVEME
	gameLocal.Printf( "RemoveShopItem( %i )\n", slotId );

	idDict tempshopSlotItem_Dict;
	int itemGroupCount;

	// Remove a shop item from the items count.
	itemGroupCount = shopSlotItem_Dict->GetInt( va( "shop_item_count_%i", slotId ), "0" );

	if ( itemGroupCount > 1 ) { // It is a stacked / grouped item

		itemGroupCount --; // Reduce slot count, grouped or not.

		shopSlotItem_Dict->SetInt( va( "shop_item_count_%i", slotId ), itemGroupCount ); // Save new slot count.

		totalUsedShopSlots = totalUsedShopSlots; // Serves no purpose - but shows the theory.

		return;
	}

	if ( itemGroupCount <= 1 ) {

		itemGroupCount --; // Reduce slot count, grouped or not.

		shopSlotItem_Dict->SetInt( va( "shop_item_count_%i", slotId ), itemGroupCount ); // Save new slot count.

		// We are now removing a slot item ( 1 shot / inventory item square is a slot )
		totalUsedShopSlots -= 1;

		// Copy the shop into a temp dictionary.
		for (int i = 0; i < ARX_MAX_SHOP_SLOTS; i++) {
			tempshopSlotItem_Dict.Set( va( "shop_item_class_%i", i ), shopSlotItem_Dict->GetString( va( "shop_item_class_%i", i ) ) );
			tempshopSlotItem_Dict.Set( va( "shop_item_icon_%i", i ), shopSlotItem_Dict->GetString( va( "shop_item_icon_%i", i ) ) );
			tempshopSlotItem_Dict.Set( va( "shop_item_name_%i", i ), shopSlotItem_Dict->GetString( va( "shop_item_name_%i", i ) ) );
			tempshopSlotItem_Dict.Set( va( "inv_shop_item_value_%i", i ), shopSlotItem_Dict->GetString( va( "inv_shop_item_value_%i", i ) ) );
			tempshopSlotItem_Dict.Set( va( "shop_item_count_%i", i ), shopSlotItem_Dict->GetString( va( "shop_item_count_%i", i ) ) );
		}

		// Clear the shop dictionary.
		shopSlotItem_Dict->Clear();

		// Copy back the items 1 by 1 ommitting the empty slots.
		int slotIndex = 0;
		for (int i = 0; i < ARX_MAX_SHOP_SLOTS; i++) {

			//REMOVEME
			gameLocal.Printf( "remshop: shop_item_class_%i = %s\n", i, tempshopSlotItem_Dict.GetString( va( "shop_item_class_%i", i ) ) );
			gameLocal.Printf( "remshop: shop_item_count_%i = %s\n", i, tempshopSlotItem_Dict.GetString( va( "shop_item_count_%i", i ) ) );

			if ( tempshopSlotItem_Dict.GetInt( va( "shop_item_count_%i", i ), "0" ) > 0 ) {

				shopSlotItem_Dict->Set( va( "shop_item_class_%i", slotIndex ), tempshopSlotItem_Dict.GetString( va( "shop_item_class_%i", i ) ) );
				shopSlotItem_Dict->Set( va( "shop_item_icon_%i", slotIndex ), tempshopSlotItem_Dict.GetString( va( "shop_item_icon_%i", i ) ) );
				shopSlotItem_Dict->Set( va( "shop_item_name_%i", slotIndex ), tempshopSlotItem_Dict.GetString( va( "shop_item_name_%i", i ) ) );
				shopSlotItem_Dict->Set( va( "inv_shop_item_value_%i", slotIndex ), tempshopSlotItem_Dict.GetString( va( "inv_shop_item_value_%i", i ) ) );
				shopSlotItem_Dict->Set( va( "shop_item_count_%i", slotIndex ), tempshopSlotItem_Dict.GetString( va( "shop_item_count_%i", i ) ) );

				slotIndex ++;
			}
		}
		totalUsedShopSlots = slotIndex; // Update the new amount of slot spaces taken up in the shop.
	}
}

int idArxShop::FindShopItem( const char *name ) {

	const char *shop_item;

	for ( int i = 0; i < ARX_MAX_SHOP_SLOTS; i++ ) {

		shop_item = shopSlotItem_Dict->GetString( va( "shop_item_class_%i", i ) );
		 
		if ( shop_item && *shop_item ) {

			//gameLocal.Printf( "(%i): name = %s and shop_item = %s\n", i, name, shop_item );

			if ( idStr::Icmp( name, shop_item ) == 0 ) {

				//gameLocal.Printf( "FindShopItem returns: %i\n", i );

				return i;
			}
		}
	}

	//gameLocal.Printf( "FindShopItem returns: -1\n" );

	return -1;
}