
#ifndef   __PLAYER_ARXSHOP_H__
#define   __PLAYER_ARXSHOP_H__

/*
Created on 8th Nov 2011 by Solarsplace
*/

//class idArxShop {

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

	private:

		int		FindShopItem			( const char *name, bool useMaxGroup );
		int		CountUsedShopSlots		( void );

};
#endif   /* !_PLAYER_ARXSHOP_H_ */
