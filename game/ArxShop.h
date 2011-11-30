
#ifndef   __PLAYER_ARXSHOP_H__
#define   __PLAYER_ARXSHOP_H__

/*
Created on 8th Nov 2011 by Solarsplace
*/

class idArxShop {

//class idArxShop : public idClass {
//{

	//CLASS_PROTOTYPE( idArxShop );

	public:

		idArxShop();
		~idArxShop();

		// save games
		void					Save( idSaveGame *savefile ) const;					// archives object for save game file
		void					Restore( idRestoreGame *savefile );					// unarchives object from save game file

		idDict					*shopSlotItem_Dict;
		int						totalUsedShopSlots;

	public:

		void	LoadActiveShop			( idEntity *shopEntity );

		void	RemoveShopItem			( int slotId );								// When items are bought from the shop
		void	AddShopItem				( const char *className );					// When items are sold to the shop

	private:

		int	FindShopItem			( const char *name );

};
#endif   /* !_PLAYER_ARXSHOP_H_ */
