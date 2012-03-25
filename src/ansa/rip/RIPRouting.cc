/**
 * @file RIPRouting.cc
 *
 * Zakladni� informace:
 *  - Author: Veronika Rybova
 *  - Contact: xrybov00@stud.fit.vutbr.cz
 *  - Date: 11.5.2009
 *  - Institutuion: Brno University of Technology
 *
 * @brief Modul RIPRouting v sobe implementuje protokol RIP a redistribuci
 * z protokolu OSPF do protokolu RIP. Tento modul vznikl v ramci me
 * bakalrske prace.
 *
 * Module RIPRouting implements protocol RIP and redistribution from
 * protocol OSPF to protocol RIP. This module was created within the scope
 * of my bachelor's thesis.
 */


#include "RIPRouting.h"
#include "IPRoute.h"
#include <vector>

#ifndef WITHOUT_IPv4
#include "IPv4InterfaceData.h"
#endif

#define BROADCAST "255.255.255.255" /**< IP adresa broadcastu. */

using namespace std;

Define_Module(RIPRouting);

/** Umoznuje vypis struktury RIPinterface. */
std::ostream& operator<<(std::ostream& os, const RIPinterface& e)
{
    os << "ID = " << e.intID << "; addr = " << e.addr << "; mask = " << e.mask << "; pass = " << e.passive << "; broad = " << e.broadcast;
    return os;
};

/** Umoznuje vypis struktury RIPRouteTimer. */
std::ostream& operator<<(std::ostream& os, const RIPRouteTimer& e)
{
	 if (e.timer == NULL)
		 os << "Adresa= " << e.route->getHost() << "; NIC ";
	 else
		 os << "Adresa= " << e.route->getHost() << "; " << e.timer->getFullName() << "  ArrivalTime: " << e.timer->getArrivalTime();
    return os;
};

/** Umoznuje vypis struktury RIPRedistribution. */
std::ostream& operator<<(std::ostream& os, const RIPRedistribution& e)
{
	 if (e.redistrinute)
		 os << "Redistribuce: YES; Protokol: " << e.protocol << "; Metrika: " << e.metric;
	 else
		 os << "Redistribuce: NO; Protokol: " << e.protocol << "; Metrika: " << e.metric;
    return os;
};

/** Destruktor uvolnuje strukturu routeTimer (RIPRouteTimer) a triggerTimer (RIPTimer).*/
RIPRouting::~RIPRouting()
{
	for (int i = 0; i < (int) routeTimer.size(); i++)
	{
		if (routeTimer[i].timer != NULL)
		{
			if (routeTimer[i].timer->isScheduled())
				cancelEvent(routeTimer[i].timer);
			delete routeTimer[i].timer;
		}
	}
	if (triggerTimer != NULL)
	{
		if (triggerTimer->isScheduled())
			cancelEvent(triggerTimer);
		delete triggerTimer;
	}
}

/**
 * GET MASK
 * Metoda slouzi k urceni masky IP adresy zarazene
 * adresy do tridy A, B nebo C.
 * @param addr IP adresa, ke ktere se hleda maska.
 * @return Maska IP adresy.
 */
IPAddress getMask(IPAddress addr)
{
	IPAddress mask;
	if (addr< "126.255.255.255")
		mask = "255.0.0.0";			// A
	else if (addr < "191.255.255.255")
		mask = "255.255.0.0";		// B
	else
		mask = "255.255.255.0";		// C
	return mask;
}

/**
 * INSERT IFT
 * Metoda slouzi k vlozeni jednoho zaznamu o jednom
 * rozhrani do struktury RIPinterface.
 * @param entryIFT Struktura popisujici rozhrani.
 */
void RIPRouting::insertIft(InterfaceEntry * entryIFT)
{
	RIPinterface newIftEntry;

	newIftEntry.intID = entryIFT->getInterfaceId();

	if (entryIFT->isBroadcast())
		newIftEntry.broadcast = true;
	else
		newIftEntry.broadcast = false;

	if (entryIFT->isLoopback())
		newIftEntry.loopback = true;
	else
		newIftEntry.loopback = false;

	IPv4InterfaceData *ipv4 = entryIFT->ipv4Data();
	IPAddress IPaddr = ipv4->getIPAddress();
	newIftEntry.addr = IPvXAddress(IPaddr);
	newIftEntry.mask = ipv4->getNetmask();
	newIftEntry.passive = false;
	newIftEntry.entry = entryIFT;
	ripIft.push_back(newIftEntry);
}

/**
 * SEND TRIGGER
 * Metoda zajistuje nastaveni casovace Trigger. Musi zkontrolovat
 * zda u� nejaky Trigger neni nastaven a pokud ano, smazat ho a
 * nastavit novy.
 */
void RIPRouting::sendTrigger()
{
	if (triggerTimer != NULL)
	{
		if (triggerTimer->isScheduled())
			cancelEvent(triggerTimer);
		delete triggerTimer;
	}
	// vysli trigger zpravu
	RIPTimer* newTimer = new RIPTimer("Trigger");
	newTimer->setTimerKind(trigger);
	triggerTimer = newTimer;
	scheduleAt(simTime() + exponential(3), newTimer);
}

/**
 * GET ROUTE RT
 * Metoda vraci index do vektoru typu RIPRouteTimer,
 * ktery v kazde polozce obsahuje ukazatel na cestu
 * a casovac, ktery ji byl prirazen. Tato metoda
 * hleda polozku podle ukazatele na casovac.
 * @param timer Casovac, ke kteremu se hleda polozka.
 * @return int Index ve vektoru RIPRouteTimer.
 */
int RIPRouting::getRouteRT (RIPTimer *timer)
{
	for (int i = 0; i < (int)routeTimer.size(); i++)
	{
		if (routeTimer[i].timer == timer)
		{
			return i;
		}
	}
	return -1;
}

/**
 * GET TIMER RT
* Metoda vraci index do vektoru typu RIPRouteTimer,
 * ktery v kazde polozce obsahuje ukazatel na cestu
 * a casovac, ktery ji byl prirazen. Tato metoda
 * hleda polozku podle ukazatele na cestu.
 * @param route Cesta, ke ktere se hleda polozka.
 * @return int Index ve vektoru RIPRouteTimer.
 */
int RIPRouting::getTimerRT (IPRoute *route)
{
	for(int i = 0; i < (int) routeTimer.size(); i++)
	{
		if (routeTimer[i].route == route)
		{
			return i;
		}
	}
	return -1;
}

/**
 * UPDATE TIMER
 * Tato metoda zajistuje tvorbu novych a obnovovani starych
 * casovacu typu Hello, Timeout a Garbage.
 * @param type Druh casovace, ktery nastavuje.
 * @param entry Polozka ve strukture RIPRouteTimer pro prirazeni
 * casovace k ceste.
 * @return Ukazatel na nove upraveny casovac.
 */
RIPTimer * RIPRouting::updateTimer(int type, RIPRouteTimer * entry)
{
	RIPTimer *timer = new RIPTimer();
	timer->setTimerKind(type);

	switch(type)
	{
		case hello:
			timer->setName("Hello");
			scheduleAt(simTime() + 30.0 + exponential(1), timer);
			break;

		case timeout:
			timer->setName("Timeout");
			scheduleAt(simTime() + 120.0, timer);
			if (entry != NULL && entry->timer != NULL)
			{
				cancelEvent(entry->timer);
				delete entry->timer;
				entry->timer = timer;
			}
			break;

		case garbage:
			timer->setName("Garbage");
			scheduleAt(simTime() + 180.0, timer);
			entry->timer = timer;
			break;
	}
	return timer;
}

/**
 * CHECK TWIN
 * Metoda zjistuje, zda existuje k dane ceste ve smerovaci
 * tabulce nejaka duplicitni cesta (ulozeni dvou cest se
 * stejnou metrikou).
 * @param entryRT IP adresa, ke ktera se hleda duplicitni.
 * @return Pokud dvojice existuje, vraci true, jinak false.
 */
bool RIPRouting::checkTwin(IPRoute * entryRT)
{
	bool cont = false;
	for (int k = 0; k < (int) routeTwins.size(); k++)
	{
		if (entryRT == routeTwins[k].route1 || entryRT == routeTwins[k].route2)
		{
			EV << hostname << ": Takováto dvojice jiz existuje!!!" << endl;
			cont = true;
		}
	}
	return cont;
}

/**
 * SEND PACKET
 * Metoda pro zasilani paketu na nizsi, tedy UDP, vrstvu.
 * @param command Typ zpravy - Response nebo Request.
 * @param destAddr Cilova IP adresa.
 * @see createPacket()
 */
void RIPRouting::sendPacket(int command, IPAddress destAddr)
{
   EV << "RIPRouting::sendPacket" << endl;
   IPvXAddress destXAddr = IPvXAddress(destAddr);
   bool right = false;
   RIPPacket *msg;

   // posilani broadcast zprav na vsechny vystupni interfacy
   if (destAddr == BROADCAST)
   {
   	EV << "Broadcast" << endl;
		for (int i = 0; i < (int)ripIft.size(); i++)
		{
			// kontrola interface, nezasilame na pasivni interfaci, jen na broadcastove
			// if (ripIft[i].passive || !ripIft[i].broadcast)
			if (ripIft[i].passive || ripIft[i].loopback)
			{
				if (ripIft[i].passive)
					EV << "FUJ" << endl;
				else if (ripIft[i].loopback)
					EV << "FUJ2" << endl;
				continue;}

			// posilame jen do siti, ktere jsou definovane v network
			for (int j = 0; j < (int) network.size(); j++)
			{
				EV << hostname << ": Kontrola na IP interfacu " << ripIft[i].addr.get4() << "s IP adresou v network: " << network[j] << " s maskou: " << getMask(network[j]) << endl;
				if (IPAddress::maskedAddrAreEqual(network[j], ripIft[i].addr.get4(), getMask(network[j])))
				{
					right = true;
					break;
				}
				else
					right = false;
			}
			// adresa neni v network
			if (right == false)
			{ EV << "FUJ2" << endl;
				continue;}

			// vytvori novou RIP zpravu
			if ((msg = createPacket(command, ripIft[i].entry)) == NULL)
				return;

			// UDP kontrolni informace (porty, IP adresy, interface)
			UDPControlInfo *ctrl = new UDPControlInfo();
			ctrl->setSrcPort(localPort);
			ctrl->setDestAddr(destXAddr);
			ctrl->setDestPort(destPort);
			ctrl->setSrcAddr(ripIft[i].addr);
			ctrl->setInterfaceId(ripIft[i].intID);
			msg->setControlInfo(ctrl);
			send(msg, "udpOut");
			EV << hostname << ": Posilam zpravu Broadcastem se zdrojem: "<< ripIft[i].addr << endl;
		}
   }
   // posilani aktualizace na konkretni IP adresu (odpoved na Request)
   else
   {
   	EV << hostname << ": Posilam zpravu na adresu:" << destAddr << endl;
   	IPRoute * route = (IPRoute *) rt->findBestMatchingRoute(destAddr);

   	// vytvori novou RIP zpravu
		if ((msg = createPacket(command, route->getInterface())) == NULL)
			return;

		// UDP kontrolni informace (porty, IP adresy, interface)
		UDPControlInfo *ctrl = new UDPControlInfo();
		ctrl->setSrcPort(localPort);
		ctrl->setDestAddr(destXAddr);
		ctrl->setDestPort(destPort);
		ctrl->setInterfaceId(route->getInterface()->getInterfaceId());
		msg->setControlInfo(ctrl);
		send(msg, "udpOut");
   }
}

/**
 * CREATE PACKET
 * Metoda pro vytvoreni RIP zpravy.
 * @param command Typ zpravy - Request nebo Response.
 * @param entry Rozhrani, na ktere se bude zprava zasilat.
 * @return Vytvorena zprava RIPPacket naplnena vsemi potrebnymi informacemi.
 * @see sendPacket()
 */
RIPPacket* RIPRouting::createPacket(int command, InterfaceEntry * entry)
{
   RIPPacket *msg = new RIPPacket();		// novy paket
   vector<RouteEntry> newEntry;				// vektor cest
   int size;								// mnozstvi cest

   // Request zprava
   if (command == Request)
   {
   	msg->setName("RIPRequest");
   	RouteEntry entry;
		entry.addressID = 0;
		entry.metric = 16;
		newEntry.push_back(entry);
   }
   // Response zprava
   else
   {
   	msg->setName("RIPResponse");
   	newEntry = fillNetworks(entry);
   }

   // kontrola, zda mame co posilat
   if (newEntry.size() <= 0)
      return NULL;

   msg->setCommand(command);
   msg->setKind(UDP_C_DATA);
   size = newEntry.size();
   msg->setRouteEntryArraySize(size);

   // naplneni zpravy cestami
   for(int i = 0; i < size; i++)
      msg->setRouteEntry(i, newEntry[i]);

   return msg;
}

/**
 * FILL ALL NETWORKS
 * Metoda slouzi k vytvoreni seznamu adres/cest, ktere se budou zasilat
 * v zpravach Response sousedum. Naplni se vsemi adresami, ktere jsou
 * v konfiguracnim souboru (element/prikaz network), a dale
 * vsemi adresami, ktere prisli od nektereho ze sousedu. Pokud je zapnuta
 * redistribuce, doplni redistribuovane cesty.
 * @param IntEntry Rozhrani, na ktere se bude zprava zasilat.
 * @return Vektor adres RouteEntry, ktere se zasilaji v RIP zprave sousedum.
 * @see createPacket()
 * @see getOSPFRoutes()
 */
vector<RouteEntry> RIPRouting::fillNetworks(InterfaceEntry * IntEntry)
{
	IPRoute *entryRT = new IPRoute();           // routovaci tabulka
	RouteEntry entry;                          	// nova polozka s adresou
	vector<RouteEntry> newEntry;                // vektor zasilanych adres
	IPAddress mask;								// sitova maska
	int sizeTab = rt->getNumRoutes();			// velikost routovaci tabulky
	int sizeNet = network.size();				// pocet siti, ktere se maji posilat


	// ber postupne site z routovaci tabulky
	for (int i = 0; i < sizeTab; i++)
	{
		// pokud cesta ma jako zdroj RIP (prisla od souseda), pridej ji...
		entryRT = const_cast<IPRoute*>(rt->getRoute(i));
		if (entryRT->getSource() == INET_API IPRoute::RIP)
		{
			// split horizon => neposilame cesty, ktere jsme pres toto roshrani dostali
			if (entryRT->getInterface() == IntEntry)
				continue;

			// kontrola na dve stejne cesty
			bool cont = false;
			for (int j = 0; j < (int) routeTwins.size(); j++)
			{
				if (entryRT == routeTwins[j].route1 || entryRT == routeTwins[j].route2)
				{
					if (IntEntry == routeTwins[j].route1->getInterface() || IntEntry == routeTwins[j].route2->getInterface())
					{
						EV << hostname << ": FillNetworks: Jedna se o dvojce, nikam ji nedavej!!!" << endl;
						cont = true;
					}
				}
			}
			if (cont == true)
				continue;

			// pridava RIP cestu
			EV << hostname << ": Posilam podle RIP zaznamu IP: " << entry.ipAddress << endl;
			entry.addressID = 2;
			entry.mustBeZero2 = 0;
			entry.mustBeZero3 = 0;
			entry.mustBeZero4 = 0;
			entry.ipAddress = entryRT->getHost();
			entry.metric = entryRT->getMetric();
			newEntry.push_back(entry);
			continue;
		} // ... konec cest se zdrojem RIP

		//... a prihod lokalni adresy, ktere jsou nastavene v konfiguraku (network)
		// ber postupne site nastavene v network a porovnavej ji s cestou z routovaci tabulky
		if (entryRT->getSource() == INET_API IPRoute::IFACENETMASK)
		{
			for (int j = 0; j < sizeNet; j++)
			{
				if (IPAddress::maskedAddrAreEqual(network[j], entryRT->getHost(), getMask(network[j])))
				{
					EV << hostname << ": Posilam IP: " << entry.ipAddress << endl;
					entry.addressID = 2;
					entry.mustBeZero2 = 0;
					entry.mustBeZero3 = 0;
					entry.mustBeZero4 = 0;
					entry.ipAddress = entryRT->getHost();
					entry.metric = entryRT->getMetric();
					newEntry.push_back(entry);
				}
			}
		} // ...konec lokalnich adres
	} // for


	// ziskej redistribuvane site
	vector<RouteEntry> redistrEntry;
	bool red;
	if (redistr.redistrinute)
	{
		EV << hostname << ": Redistribuce zapnuta" << endl;
		// ziskani cest z OSPF
		redistrEntry = getOSPFRoutes();

		// OSPF dava do routovaci tabulky i primo pripojene site
		// ty je nutne vyeliminovat
		for(int j = 0; j < (int) redistrEntry.size(); j++)
		{
			red = true;
			for (int i = 0; i < (int) ripIft.size(); i++)
			{
				if (IPAddress::maskedAddrAreEqual(ripIft[i].addr.get4(), redistrEntry[j].ipAddress, ripIft[i].mask.get4()))
					red = false;
			}
			if (red)
				newEntry.push_back(redistrEntry[j]);
		}
	}// ...if redistribuce

	return newEntry;
}


/**
 * PROCESS REQUEST
 * Metoda zpracovavajici RIP zpravu typu Request, tedy zadost o smerovaci
 * informace. Zjistuje, zda zadatel chce poslat informace o vsech cestach nebo
 * se zajima jen o konkretni cesty. Podle toho necha vytvorit zpravu.
 * @param msg RIP zprava.
 * @see sendPacket()
 * @see processPacket()
 */
void RIPRouting::processRequest(RIPPacket *msg)
{
	EV << "RIPRouting::processRequest" << endl;
   RouteEntry entryReq = msg->getRouteEntry(0);    // 1. zaznam z prichozi zpravy

   if ((msg->getRouteEntryArraySize() == 1) && (entryReq.addressID == 0) && (entryReq.metric == 16))
   {
      // ...posli aktualizacni zaznam se vsemi cestami
   	this->sendPacket(Response, udpCtrl->getSrcAddr().get4());
   }
   else
   {
      // ... jinak posli jen pozadovane zaznamy
   	// mozno udelat pri nejakem rozsireni, ale pri simulaci
   	// prakticky nepouzitelne; pouziva se k siagnostickym ucelum
   }
}

/**
 * PROCESS RESPONSE
 * Metoda zpracovavajici RIP zpravu typu Response, tedy zpravu nesouci smerovaci
 * informace od souseda. Kontroluje cesty, ktere prisli, pridava nove cesty do
 * smerovaci tabulky, upravuje stavajici cesty, prodluzuje casovace.
 * @param msg RIP zprava.
 * @see processPacket()
 */
void RIPRouting::processResponse(RIPPacket *msg)
{
   // zkontroluj src port 520 (RIP), jinak ignoruj
   if ((udpCtrl->getSrcPort()) != destPort)
      return;

   int max = msg->getRouteEntryArraySize();     // pocet prijatych cest
   IPRoute *entryRT = new IPRoute();            // zaznam z routovaci tabulky
   int newMetric;                               // metrika ze zpravy
   int oldMetric;                               // metrika z routovaci tabulky
   bool change = false;							// hlida zmeny, true = trigged zprava

   // prochazej vsechny prijate zaznamy
   for (int i = 0; i < max; i++)
   {
   	// pokud nejde o IP adresu, preskoc zaznam
      if  (msg->getRouteEntry(i).addressID != 2)
         continue;

      // pokud je metrika vetsi nez infinite, preskoc zaznam
      if (msg->getRouteEntry(i).metric >= 16)
      	continue;

      // loopbacky taky ignoruj
      if (IPAddress::maskedAddrAreEqual(msg->getRouteEntry(i).ipAddress, "127.0.0.0", "255.0.0.0"))
      	continue;

      // adresy tridy D a E ignoruj taktez
      if (!(msg->getRouteEntry(i).ipAddress < "223.255.255.255"))
      	continue;

      // najdi IP adresu v routovaci tabulce
      EV << hostname << ": Adresa " << i << ": "<< msg->getRouteEntry(i).ipAddress << endl;
      entryRT = const_cast<IPRoute*>(rt->findBestMatchingRoute(msg->getRouteEntry(i).ipAddress));

      // pro exitující twin
      if (checkTwin(entryRT))
      {
      	EV << hostname << "Existuje dvojice, hledam routu" << endl;
      	for (int k = 0; k < (int) routeTwins.size(); k++)
			{
				if (entryRT == routeTwins[k].route1 || entryRT == routeTwins[k].route2)
				{
					EV << hostname << "Routa nalezena: " << endl;
					if (udpCtrl->getSrcAddr().get4() == routeTwins[k].route1->getGateway())
					{
						entryRT = routeTwins[k].route1;
						EV << entryRT->info() << endl;
					}
					else if (udpCtrl->getSrcAddr().get4() == routeTwins[k].route2->getGateway())
					{
						entryRT = routeTwins[k].route2;
						EV << entryRT->info() << endl;
					}
					break;
				}
			}
      } //if twin

      // pokud adresa neni v routovaci tabulce, je treba pridat novy zaznam
      if (entryRT == NULL || ((entryRT->getGateway() != udpCtrl->getSrcAddr().get4()) && (msg->getRouteEntry(i).metric + 1 == entryRT->getMetric()) && !checkTwin(entryRT)))
      {
      	IPRoute *entry = new IPRoute();	// nova cesta
      	RIPRouteTwins twin;					// dvojice stejnych cest
			int id;

			// zjisti z jakeho interfacu zprava prisla
			// kvuli urceni masky a adresy cesty
      	for (int j = 0; j < (int) ripIft.size(); j++)
				if (ripIft[j].intID == udpCtrl->getInterfaceId())
				{
					id = j;
					break;
				}

      	// pokud je adresa ze stejne tridy dostane masku podle masky na interfacu
      	if (IPAddress::maskedAddrAreEqual(msg->getRouteEntry(i).ipAddress, ripIft[id].addr.get4(), getMask(msg->getRouteEntry(i).ipAddress)))
      		entry->setNetmask(ripIft[id].mask.get4());
      	else
      		entry->setNetmask(getMask(msg->getRouteEntry(i).ipAddress));

      	// vyplni vsechny potrebne informace a vlozi cestu do tabulky
      	entry->setHost(msg->getRouteEntry(i).ipAddress);
      	entry->setGateway(udpCtrl->getSrcAddr().get4());
      	entry->setInterface(ift->getInterfaceById(udpCtrl->getInterfaceId()));
      	entry->setType(INET_API IPRoute::REMOTE);
      	entry->setSource(INET_API IPRoute::RIP);
      	entry->setMetric(msg->getRouteEntry(i).metric + 1);
      	rt->addRoute(entry);
      	EV << "Pridavam adresu: "<< msg->getRouteEntry(i).ipAddress << endl;

      	// jde o druhy zaznam stejne cesty
      	if (entryRT != NULL)
      	{
      		EV << hostname << ": Pridavam dvojcata: " << endl;
      		twin.route1 = entryRT;
      		twin.route2 = entry;
      		EV << twin.route1->info() << endl;
      		EV << twin.route2->info() << endl;
      		routeTwins.push_back(twin);
      	}
			// zaznam do RouteEntry + nastavi timer timeout
      	RIPRouteTimer newEntry;
      	newEntry.route = entry;
      	newEntry.timer = updateTimer(timeout, NULL);
      	routeTimer.push_back(newEntry);
      }
      // adresa jiz v routovaci tabulce je a nejedna se o druhou cestu do stejného místa
      else
      {
      	EV << hostname << ": K teto adrese sedi adresa: " << entryRT->getHost() << endl;
			newMetric = msg->getRouteEntry(i).metric;
			oldMetric = entryRT->getMetric();
			int j = getTimerRT(entryRT);

			// updatuj timery
			if (entryRT->getGateway() == udpCtrl->getSrcAddr().get4())
				updateTimer(timeout, &routeTimer[j]);

			// pokud je nova metrika mensi nez metrika v tabulce,
			// prepis ji a zapis novou adresu gateway (next hop)
			if (((newMetric + 1) < oldMetric))
			{
				EV << "Stara metrika: " << oldMetric << ". Nova metrika: " << newMetric << endl;
				entryRT->setMetric(newMetric + 1);
				entryRT->setGateway(udpCtrl->getSrcAddr().get4());

				// nastavi upravene ceste spravny odkaz na interface
				for (int k = 0; k < (int) ripIft.size(); k++)
				{
					if (udpCtrl->getInterfaceId() == ripIft[k].intID)
					{
						entryRT->setInterface(ripIft[k].entry);
						break;
					}
				}
				// vysle updaty
				change = true;
			}
      } // else
   } // for

   // vysle updat
   if (change == true)
   	sendTrigger();
}

/**
 * PROCESS PACKET
 * Metoda, ktera zpracuje prijatou zpravu od UDP modulu. Zkontroluje pravost
 * zpravy dle IP adresy odesilatele. Zkontroluje, jestli je zprava v poradku.
 * Podle toho, zda se jedna o Request ci Response, posle zpravu k dalsimu
 * zpracovani.
 * @param msg Zprava, kterou RIPRouting prijme od UDP modulu.
 * @see handleMessage()
 * @see processRequest()
 * @see processResponse()
 */
void RIPRouting::processPacket(cMessage *msg)
{
   int max;													// pocet cest v prichozi zprave
   cPacket *packet = dynamic_cast<cPacket *> (msg);			// pretipujeme na cPacket
   udpCtrl = new UDPControlInfo();							// naalokujeme misto pro UDP kontrolni info
   udpCtrl = dynamic_cast<UDPControlInfo *>(packet->getControlInfo());
   IPvXAddress srcIP = udpCtrl->getSrcAddr();				// IP adresa zdroje
   RIPPacket *hMsg = dynamic_cast<RIPPacket *> (msg);		// pretipujeme na RIP zpravu
   IPRoute * route = new IPRoute();							// naalokujeme cestu z RT
   EV << hostname << ": IP adresa zdroje RIP zpravy je:" << srcIP << endl;

   // zkontroluj IP adresu zdroje
   // sit zdroje neni v routovaci tabulce nebo neni primo pripojena
   if ((route = const_cast<IPRoute*> (rt->findBestMatchingRoute(srcIP.get4()))) == NULL)
   {
      error("Nejspise se jedna o podvrzenou zpravu, chybna zdrojova IP adresa1");
      return;
   }
   else if (route->getType() !=  INET_API IPRoute::DIRECT)
   {
      error("Nejspise se jedna o podvrzenou zpravu, chybna zdrojova IP adresa2");
      return;
   }

   // zkontroluj verzi a prislusna policka na spravnost
   switch(hMsg->getVersion())
   {
      case 0:
         return;
      case 1:
         if (!(hMsg->getMustBeZero1() == 0))
            return;
         max = hMsg->getRouteEntryArraySize();

         for (int i = 0; i < max; i++)
         {
            if  (!(hMsg->getRouteEntry(i).mustBeZero2 == 0))
               return;
            if  (!(hMsg->getRouteEntry(i).mustBeZero3 == 0))
               return;
            if  (!(hMsg->getRouteEntry(i).mustBeZero4 == 0))
               return;
          }
         break;
      default:
         break;
   }

   // dale zpracovavej podle toho, zda se jedna o Request nebo Response
   if (hMsg->getCommand() == Request)
      this->processRequest(hMsg);
   else if (hMsg->getCommand() == Response)
      this->processResponse(hMsg);
   delete msg;
}

/**
 * INITIALIZE
 * Metoda incializuje smerovani protokolem RIP. Nejprve se aplikace pribinduje
 * na lokalni UDP RIP port. Dale zapne casovac Hello, ziska pristup k smerovaci
 * tabulce a tabulce rozhrani, ze ktere vytvori vlastni tabulku rozhrani pro
 * interni pouziti. Hlavne nacte konfiguraci z konfiguracniho souboru.
 * @see insertIft()
 * @see LoadConfigFromXML()
 */
void RIPRouting::initialize (int stage)
{
   // v stage 2 se registruji interfacy
   if (stage == 3)
   {
	   // nastaveni portu a spojeni s portem
	   localPort = par("localPort");
	   destPort = par("destPort");
	   bindToPort(localPort);

	   // routovaci tabulka, tabulka interfacu
	   // notifikacni tabulka a odebirani upozorneni
	   rt = RoutingTableAccess().get();
	   ift = InterfaceTableAccess().get();
	   nb = NotificationBoardAccess().get();
	   nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);

	   // dalsi inicializace
	   triggerTimer = NULL;
	   redistr.redistrinute = false;
	   redistr.metric = 0;

      // vlastni tabulka interfacu s info, ktere potrebuji
      InterfaceEntry *entryIFT = new InterfaceEntry();
      for (int i = 0; i < ift->getNumInterfaces(); i++)
      {
      	entryIFT = ift->getInterface(i);
      	insertIft(entryIFT);
      }

      // zpracovani XML konfigurace pro RIP
      hostname = par("hostname");
		const char *fileName = par("configFile");
		if (!LoadConfigFromXML(fileName))
			return;

		// sledování klíčových proměnných
		WATCH_VECTOR(ripIft);
		WATCH_VECTOR(network);
		WATCH_VECTOR(routeTimer);
		WATCH(redistr);

      // zapni hello timer
      RIPTimer *msg = new RIPTimer("Hello");
      msg->setTimerKind(hello);
      scheduleAt(simTime() + 30.0 + exponential(1), msg);

      // vysli requesty vsem sousedum
      this->sendPacket(Request, BROADCAST);
   }
}

/**
 * HANDLE MESSAGE
 * Metoda, ktera zpracuje prijatou zpravu. Nejprve zjisti, jestli jde o vlastni
 * zpravu nebo zpravu z UDP modulu a podle toho se zachova. U vlastni zpravy
 * zjisti, o jaky typ se jedna a podle toho jej zpracuje. Pokud se jedna o
 * zpravu z jineho smerovace, odesle ji jine metode k zpracovani.
 * @param msg Zprava, ktera prisla tride
 * @see processPacket()
 */
void RIPRouting::handleMessage(cMessage *msg)
{
	EV << hostname << "RIPRouting::handleMessage" << endl;
   vector<RouteEntry> newEntry;
   IPAddress mask;
   int size;
   int i;

   // vlastni zprava, tj. nektery z casovacu
   if (msg->isSelfMessage())
   {
   	RIPTimer *timer = check_and_cast <RIPTimer *> (msg);
   	switch(timer->getTimerKind())
      {
			case hello:
            // hello packet (30s)
         	EV << hostname << ": Prisel hello." << endl;
            this->sendPacket(Response, BROADCAST);
            updateTimer(hello, NULL);
            delete timer;
            break;

			case timeout:
            // vyprsel garbage-collection timer (120s)
         	EV << hostname << ": Prisel timeout." << endl;
         	i = getRouteRT(timer);
         	routeTimer[i].route->setMetric(16);
         	updateTimer(garbage, &routeTimer[i]);
         	sendTrigger();
         	delete timer;
            break;

			case garbage:
            // vymaz cestu, timout (180s)
         	EV << hostname << ": Prisel garbage." << endl;
         	i = getRouteRT(timer);
         	EV << hostname << ": Vyprsel garbage na adrese: " << routeTimer[i].route->getHost() << endl;
         	EV << hostname << ": Mazu cestu: " << routeTimer[i].route->info() << endl;
				rt->deleteRoute(routeTimer[i].route);
				routeTimer.erase(routeTimer.begin() + i);
         	delete timer;
            break;

         case trigger:
				// hello packet (30s)
				EV << "Prisel trigger." << endl;
				size = rt->getNumRoutes();
				this->sendPacket(Response, BROADCAST);
				delete triggerTimer;
				triggerTimer = NULL;
				break;

         default:
         	delete timer;
      }
   }
   // cizi RIP zprava
   else if (dynamic_cast<RIPPacket *>(msg))
   	this->processPacket(msg);
   else
   	EV << "ICMP ERROR nebo jina zprava" << endl;
}

/**
 * LOAD CONFIG FROM XML
 * Tato metoda zajisti nacteni nastaveni protokolu RIP z konfiguracniho
 * souboru. Nacita site, ktere se budou propagovat a pasivni
 * rozhrani. Zjistuje take, zda je nastavena redistribuce a cte pro
 * ni protokol a metriku.
 * @param filename Nazev konfiguracniho souboru.
 * @return Vraci true, pokud nacteni probehlo v poradku, jinak vraci false.
 * @see initialize()
 */
bool RIPRouting::LoadConfigFromXML(const char *filename)
{
	// nacteni dokumentu
   cXMLElement* asConfig = ev.getXMLDocument(filename);
   if (asConfig == NULL)
       return false;

   // prvni element <Router id="192.168.10.7">
   std::string routerXPath("Router[@id='");
   IPAddress routerId = rt->getRouterId();
   routerXPath += routerId.str();
	routerXPath += "']";

	cXMLElement* routerNode = asConfig->getElementByPath(routerXPath.c_str());
	if (routerNode == NULL)
	{
		error("No configuration for Router ID: %s", routerId.str().c_str());
	   return false;
	}

	cXMLElement* routingNode = routerNode->getElementByPath("Routing");
	if (routingNode == NULL)
		 return false;

   cXMLElement* ripNode = routingNode->getElementByPath("Rip");
   if (ripNode == NULL)
       return false;

   // vypis siti, ktere se maji propagovat v RIP zpravach
   cXMLElementList childrenNodes = ripNode->getChildrenByTagName("Network");
   if (childrenNodes.size() > 0)
   {
      for (cXMLElementList::iterator node = childrenNodes.begin(); node != childrenNodes.end(); node++)
      {
         network.push_back(ULongFromAddressString((*node)->getNodeValue()));
      }
   }
   else
      return false;

   // vypis pasivnich interfacu, na ktere se nemaji zasilat RIP zpravy
   childrenNodes = ripNode->getChildrenByTagName("Passive-interface");
	if (childrenNodes.size() > 0)
	{
		for (cXMLElementList::iterator node = childrenNodes.begin(); node != childrenNodes.end(); node++)
		{
			int id = ift->getInterfaceByName((*node)->getNodeValue())->getInterfaceId();
			for (int i = 0; i < (int) ripIft.size(); i++)
			{
				if (ripIft[i].intID == id)
				{
					ripIft[i].passive = true;
					break;
				}
			}
		}
	}

	// nacteni elementu redistribuce
	cXMLElement* redistrNode = ripNode->getElementByPath("Redistribute");
	if (redistrNode == NULL)
		return true;

	// nacteni protokolu, ze ktereho se bude redistribuvota (ospf)
	cXMLElement* protocolNode = redistrNode->getElementByPath("Protocol");
	if (protocolNode == NULL)
	{
		 error("Pro redistribuci neni definovan protokol.");
		 return false;
	}
	else
		redistr.protocol = (char *) protocolNode->getNodeValue();

	// nacteni metriky, ktera se priradi cizim cestam
	cXMLElement* metricNode = redistrNode->getElementByPath("Metric");
	if (metricNode == NULL)
	{
		 error("Pro redistribuci neni definovana metrika.");
		 return false;
	}
	else
		redistr.metric = atol(metricNode->getNodeValue());

   redistr.redistrinute = true;
   return true;
}

/**
 * RECEIVE CHANGE NOTIFICATION
 * Tato metoda zajisuje reakci na zmeny, ktere zasila Notification Table.
 * V teto tride zajistuje reakci na vypnuti/zapnuti rozhrani na smerovaci.
 * V pripade, ze dojde k vypnuti rozhrani, musi metoda smazat prime site,
 * ktere byly dosazitelne na tomto rozhrani a rozeslat aktualizace sousedum.
 * V pripade zapnuti rozhrani, musi primo pripojene site na toto rozhrani,
 * znovu zavest do smerovaci tabulky, a rozeslat aktualizace.
 * @param category Typ upozorneni.
 * @param details Objekt, ktery vyvolal notifikaci (v tomto pripade ukazatel na rozhrani).
 */
void RIPRouting::receiveChangeNotification(int category, const cPolymorphic *details)
{
	// ignoruj notifikaci behem inicializace
   if (simulation.getContextType() == CTX_INITIALIZE)
       return;

   Enter_Method_Silent();
   printNotificationBanner(category, details);


   // notifikace na rozhrani
   if (category == NF_INTERFACE_STATE_CHANGED)
   {
       InterfaceEntry *entry = check_and_cast<InterfaceEntry*>(details);
       int i = 0;

		 // Interface je DOWN
       if (entry->isDown())
       {
      	 EV << hostname << ": Interface je DOWN\n";

      	 // vymaz primou cestu z tabulky a uloz ji do struktury intDown
      	 for (i = 0; i < (int) rt->getNumRoutes(); i++)
      	 {
      		 if (rt->getRoute(i)->getType() == INET_API IPRoute::DIRECT)
      		 {
      			 if (rt->getRoute(i)->getInterface() == entry)
      			 {
      				 EV << "Mazu cestu: " << rt->getRoute(i)->info() << endl;
      				 IPRoute *entryRT = new IPRoute();
      				 //entryRT = const_cast<IPRoute*> (rt->getRoute(i));
      				 entryRT->setHost(rt->getRoute(i)->getHost());
      				 entryRT->setGateway(rt->getRoute(i)->getGateway());
      				 entryRT->setNetmask(rt->getRoute(i)->getNetmask());
						 entryRT->setInterface(entry);
						 entryRT->setType(INET_API IPRoute::DIRECT);
						 entryRT->setSource(INET_API IPRoute::IFACENETMASK);
						 entryRT->setMetric(1);
      				 rt->deleteRoute(rt->getRoute(i));
      				 intDown.push_back(entryRT);
      			 }
      		 }
      	 }

      	 // vymaz cestu i z interni tabulky rozhraní ripIft
      	 for (i = 0; i < (int) ripIft.size(); i++)
      	 {
      		 if (ripIft[i].entry == entry)
      			 ripIft.erase(ripIft.begin() + i);
      	 }

      	 // vzdalene cesty smaz taky
      	 for(i = 0; i < (int) routeTimer.size(); i++)
      	 {
				 if (routeTimer[i].route->getInterface() == entry)
				 {
					 EV << "Mazu cestu: " << routeTimer[i].route->info() << endl;

					 // zrusit a samzat jeji casovac
					 if (routeTimer[i].timer->isScheduled())
						 cancelEvent(routeTimer[i].timer);
					 delete routeTimer[i].timer;

					 // v pripade, ze cesta má dvojici, vymaz zaznam z checkTwin
					 if (checkTwin(routeTimer[i].route))
					 {
						 for (int j = 0; j < (int) routeTwins.size(); j++)
						 {
							 if (routeTimer[i].route == routeTwins[j].route1 || routeTimer[i].route == routeTwins[j].route2)
								 routeTwins.erase(routeTwins.begin() + j);
						 }
					 }
					 // vymazat cestu a jeji znaznam v routeTimer
					 rt->deleteRoute(routeTimer[i].route);
					 routeTimer.erase(routeTimer.begin() + i);
					 i--;
				 }
			 }
       }
       // Interface je UP
       else
       {
      	 EV << hostname << ": Interface je UP\n";

      	 // vlozime zpet do RT prime cesty
      	 for(i = 0; i < (int) intDown.size(); i++)
      	 {
      		 if (intDown[i]->getInterface() ==  entry)
      		 {
      			 rt->addRoute(intDown[i]);
      			 intDown.erase(intDown.begin() + i);
      		 }
      	 }
      	 // vloz zaznam do ripIft
      	 insertIft(entry);
       }
       // rozesli aktualizace
       sendTrigger();
    }
}

/**
 * GET OSPF ROUTES
 * Tato metoda implementuje redistribuci z OSPF do RIPu. V smerovaci tabulce
 * vyhleda vsechny cesty jejichz zdroj je OSPF. Ty vlozi do struktury cest,
 * ktere se budou zasilat v aktualizacich, a priradi jim metriku uvedenou
 * v konfiguracnim souboru.
 * @return Vraci vektor cest RouteEntry, ktere se budou zasilat v aktualizacich sousedum.
 * @see fillNetworks()
 */
vector<RouteEntry> RIPRouting::getOSPFRoutes()
{
	// redistribujeme z protokolu OSPF?
	vector<RouteEntry> newEntry;				// vektor zasilanych adres
	if (strcmp(redistr.protocol, "ospf"))
		return newEntry;

	// proměnné
	IPRoute *entryRT = new IPRoute();           // routovaci tabulka
	int sizeTab = rt->getNumRoutes();			// velikost routovací tabulky
	RouteEntry entry;                          	// nova polozka s adresou

	// ber postupne site z routovaci tabulky a kontroluj je
	for (int i = 0; i < sizeTab; i++)
	{
		entryRT = const_cast<IPRoute*>(rt->getRoute(i));
		if (entryRT->getSource() == INET_API IPRoute::OSPF)
		{
			// pridava RIP cestu
			entry.addressID = 2;
			entry.mustBeZero2 = 0;
			entry.mustBeZero3 = 0;
			entry.mustBeZero4 = 0;
			entry.ipAddress = entryRT->getHost();
			EV << "Posilam podle RIP zaznamu IP: " << entry.ipAddress << endl;
			entry.metric = redistr.metric;
			newEntry.push_back(entry);
			continue;
		}
	}

	return newEntry;
}
