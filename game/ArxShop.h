
#ifndef   __PLAYER_ARXSHOP_H__
#define   __PLAYER_ARXSHOP_H__

/*
Created on 8th Nov 2011 by Solarsplace
*/

//class idArxShop {

typedef enum {
	ARX_SHOP_ALL		= BIT(0),	// 1 - Anything
	ARX_SHOP_GEMS		= BIT(1),	// 2 - Gemstones
	ARX_SHOP_WEAPONS	= BIT(2),	// 4 - Weapons, armour, arrows, pickaxe
	ARX_SHOP_FOOD		= BIT(3),	// 8 - All food & wine, health plants
	ARX_SHOP_MAGIC		= BIT(4)	// 16 - Scrolls, potions, mana plants, runes
} arx_shopFlags_t;

class idArxShop : public idClass
{
	public:

		CLASS_PROTOTYPE( idArxShop );

		idArxShop();
		~idArxShop();

		idDict							*shopSlotItem_Dict;
		int								totalUsedShopSlots;
		idStr							currentShopName;

		float							ratioSellToPlayer;
		float							ratioBuyFromPlayer;

	public:

		void	LoadActiveShop			( idEntity *shopEntity );
		void	SaveShopState			( void );									// Persist shop contents

		void	RemoveShopItem			( int slotId );								// When items are bought from the shop
		bool	AddShopItem				( const char *className );					// When items are sold to the shop
		void	UpdateShopPersistInfo	( void );									// After buying or selling persist the new state
		bool	MatchShopFlags			( int itemFlags );

	private:

		int		FindShopItem			( const char *name, bool useMaxGroup );
		int		CountUsedShopSlots		( void );

		int		currentShopFlags;

};
#endif   /* !_PLAYER_ARXSHOP_H_ */
