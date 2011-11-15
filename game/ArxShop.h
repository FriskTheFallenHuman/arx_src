
#ifndef   __PLAYER_ARXSHOP_H__
#define   __PLAYER_ARXSHOP_H__

/*
Created on 8th Nov 2011 by Solarsplace
*/

class idArxShop {

	public:
	idArxShop();
	~idArxShop();

	public:

		idDict					*shopSlotItem_Class;

	public:

		void	LoadActiveShop			( idEntity *shopEntity );

	private:

		int	FindShopItem			( const char *name );

};
#endif   /* !_PLAYER_ARXSHOP_H_ */
