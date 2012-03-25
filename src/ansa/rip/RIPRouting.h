/**
 * @class RIPRouting RIPRouting.h
 * @brief Třída zajišťující směrování pomocí protokolu RIP a
 *  redistribuci z OSPF do RIP. 
 * 
 * Základní informace: 
 *  - Author: Veronika Rybová
 *  - Contact: xrybov00@stud.fit.vutbr.cz
 *  - Date: 11.5.2009
 *  - Institutuion: Brno University of Technology
 *
 * @file RIPRouting.h 
 * @brief  
 * Hlavičkový soubor modulu RIPRouting, který v sobě implementuje 
 * protokol RIP a redistribuci
 * z protokolu OSPF do protokolu RIP. Tento modul vznikl v rámci mé 
 * bakalářské práce.
 * 
 * Module RIPRouting implements protocol RIP and redistribution from
 * protocol OSPF to protocol RIP. This module was created within the 
 * scope of my bachelor's thesis.
 */

#ifndef HLIDAC_RIPROUTING
#define HLIDAC_RIPROUTING

#include <omnetpp.h>
#include "UDPAppBase.h"
#include "RIPRouting.h"
#include "RIPPacket_m.h"
#include "RIPTimer_m.h"
#include "RoutingTableAccess.h"
#include "IRoutingTable.h"
#include "IPRoute.h"
#include "UDPControlInfo_m.h"
#include "OSPFcommon.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "ICMPMessage_m.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"
#include "InterfaceStateManager.h"

class IPv4InterfaceData;

/**
 * @brief 
 * Struktura představuje interní tabulku rozhraní se
 * všemi informacemi, které RIP potřebuje. 
 */ 
struct RIPinterface
{
	int 				intID;          /**< Identifikátor rozhraní. */
	IPvXAddress 	addr;           /**< IP adresa na rozhraní. */
	IPvXAddress 	mask;           /**< Maska na rozhraní. */
	bool 				broadcast;      /**< Umožňuje broadcast? */
	bool 				loopback;       /**< Jde o loopback? */
	bool				passive;        /**< Můžou se na rozhraní zasílat RIP zprávy ?*/
	InterfaceEntry*entry;          /**< Struktura popisující rozhraní. */
};

/**
 * @brief 
 * Struktura spojuje cestu ze směrovací tabulky s časovačem protokolu RIP.
 */ 
struct RIPRouteTimer
{
	IPRoute * 	route;            /**< Cesta ze směrovací tabulky. */
	RIPTimer * 	timer;            /**< Časovač přiřazený k cestě. */
};

/**
 * @brief 
 * Struktura zaznamenává dvě různé cesty do jedné sítě.
 */ 
struct RIPRouteTwins
{
	IPRoute * 	route1;           /**< První cesta.*/
	IPRoute * 	route2;           /**< Druhá cesta. */
};

/**
 * @brief 
 * Struktura nese podstatné informace o redistribuci.
 * 
 * Jedná se o redistribuovaný protokol a metriku RIP.  
 */ 
struct RIPRedistribution
{
	char *		protocol;         /**< Redistribuovaný protokol (např. OSPF).*/
	int 			metric;           /**< Metrika nových RIP cest. */
	bool			redistrinute;     /**< Je zapnuta redistribuce? */
};

class RIPRouting: public UDPAppBase, protected INotifiable
{
   private:
   	std::vector<RIPRouteTimer>	routeTimer;    /**< Spojení časovačů s cestami. */
      std::vector<IPAddress>  	network;       /**< Interní sítě, které se mají propagovat. */
      std::vector<RIPinterface> 	ripIft;        /**< Interní tabulka rozhraní. */
      std::vector<IPRoute *>		intDown;       /**< Interní cesty, které byly odstraněny při pádu linky. */
      std::vector<RIPRouteTwins> routeTwins;    /**< Dvojice cest do stejné sítě. */
      RIPTimer *						triggerTimer;  /**< Časovač Trigger. */
      IRoutingTable           	*rt;           /**< Odkaz na směrovací tabulku. */
      IInterfaceTable         	*ift;          /**< Odkaz na tabulku rozhraní. */
      NotificationBoard 			*nb;           /**< Odkaz na notifikační tabulku. */
      UDPControlInfo          	*udpCtrl;      /**< Kontrolní UDP informace příchozí zprávy.*/
      int                     	localPort;     /**< Lokální UDP port protokolu RIP. */
      int                     	destPort;      /**< Vzdálený UDP port protokolu RIP. */
      const char *					hostname;      /**< Název směrovače. */
      RIPRedistribution				redistr;       /**< Redistribuční informace. */

      // zpracovaní RIP zpráv
      void processPacket(cMessage *msg);
      void processRequest(RIPPacket *msg);
      void processResponse(RIPPacket *msg);

      // odesílání a tvorba RIP zpráv
      void sendPacket(int command, IPAddress destAddr);
      RIPPacket* createPacket(int command, InterfaceEntry * entry);
      std::vector<RouteEntry> fillNetworks(InterfaceEntry * entry);

      // nacteni konfigurace, redistribuce, notifikace
      bool LoadConfigFromXML(const char * filename);
      std::vector<RouteEntry> getOSPFRoutes();
      void receiveChangeNotification(int category, const cPolymorphic *details);

      // pomocné metody
      void sendTrigger();
      bool checkTwin(IPRoute * entryRT);
      RIPTimer * updateTimer(int type, RIPRouteTimer * entry);
      int getRouteRT (RIPTimer *timer);
      int getTimerRT (IPRoute *route);
      void insertIft(InterfaceEntry * entryIFT);

   protected:
      virtual int numInitStages() const  {return 4;}
      virtual void handleMessage(cMessage *msg);
      virtual void initialize(int stage);

   public:
      virtual ~RIPRouting();
};

#endif
