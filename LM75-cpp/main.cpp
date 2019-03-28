#include "mbed.h"
	#include "zest-radio-atzbrf233.h"
	#include "MQTTNetwork.h"
	#include "MQTTmbed.h"
	#include "MQTTClient.h"
	#include "rtos.h"


	const char* dashboard_host = "io.adafruit.com";
	const int dashboard_port = 1883;
	char* dashboard_id = "SLBORDAS";
	char* dashboard_Key = "03b3fa8e8cd043f8b91f9a46c5729073";

	const char* topicEtatLed = "/SLBORDAS/feeds/ETAT_LED";
	const char* topicGraphTemp = "/SLBORDAS/feeds/GRAPH_TEMP";
	const char* topicGraphHum = "/SLBORDAS/feeds/GRAPH_HUMI";


	static Ticker ticker;

	//définit les fonctions
	static void yieldAuto(MQTT::Client<MQTTNetwork, Countdown> *c);						//ping le serveur
	static void every_Seconds(MQTT::Client<MQTTNetwork, Countdown> *c);					//réalise les taches
	static float Recup_Temperature_Capteur();		//recupere la temp
	static float Recup_Humidite_Capteur();			//recupere l'hum
	static void sendDatas(float t, float h, MQTT::Client<MQTTNetwork, Countdown> *c);
	static void test();

	void test(){
		 printf("alive \n");
	}

	void yieldAuto(MQTT::Client<MQTTNetwork, Countdown> *c){
		c->yield(100);
	}

	void every_Seconds(MQTT::Client<MQTTNetwork, Countdown> *c){

		//récupere temp et hum
		float temperature = Recup_Temperature_Capteur();
		float humidite = Recup_Humidite_Capteur();

		//envois les données sur le serveur
		sendDatas(temperature, humidite, c);

		//affichage sur console
		printf("%f \n", temperature);
		printf("%f \n", humidite);
		printf("\n");

	}

	float Recup_Temperature_Capteur(){

		// récupere la tension du capteur

		// conversion

		//retourne la valeur
		return 25.0;
	}

	float Recup_Humidite_Capteur(){

		// récupere la tension du capteur

		// conversion

		// retourne la valeur
		return 51.5;
	}

	void sendDatas(float temperature, float humidite, MQTT::Client<MQTTNetwork, Countdown> *c){
		//rc = client.publish(topicGraphTemp, message);
	}


	// Network interface
	NetworkInterface *net;

	int arrivedcount = 0;

	/* Printf the message received and its configuration */
	void messageArrived(MQTT::MessageData& md)
	{
		MQTT::Message &message = md.message;
		printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
		printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
		++arrivedcount;
	}

	// MQTT demo
	int main() {

		EventQueue event_queue;
		int result;

		printf("Start\n");

		// Add the border router DNS to the DNS table
		nsapi_addr_t new_dns = {
			NSAPI_IPv6,
			{ 0xfd, 0x9f, 0x59, 0x0a, 0xb1, 0x58, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x01 }
		};
		nsapi_dns_add_server(new_dns);

		// Get default Network interface (6LowPAN)
		net = NetworkInterface::get_default_instance();
		if (!net) {
			printf("Error! No network interface found.\n");
			return 0;
		}

		// Connect 6LowPAN interface
		result = net->connect();
		if (result != 0) {
			printf("Error! net->connect() returned: %d\n", result);
			return result;
		}

		// Build the socket that will be used for MQTT
		MQTTNetwork mqttNetwork(net);

		// Declare a MQTT Client
		MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

		// Connect the socket to the MQTT Broker
		const char* hostname = dashboard_host;
		uint16_t port = dashboard_port;
		printf("Connecting to %s:%d\r\n", hostname, port);
		int rc = mqttNetwork.connect(hostname, port);
		if (rc != 0)
			printf("rc from TCP connect is %d\r\n", rc);

		// Connect the MQTT Client
		MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
		data.MQTTVersion = 3;
		data.clientID.cstring = "mbed-sample";
		data.username.cstring = dashboard_id;
		data.password.cstring = dashboard_Key;
		if ((rc = client.connect(data)) != 0)
			printf("rc from MQTT connect is %d\r\n", rc);

		// Subscribe to the same topic we will publish in

		/*
		if ((rc = client.subscribe(topicEtatLed, MQTT::QOS2, messageArrived)) != 0)
			printf("rc from MQTT subscribe is %d\r\n", rc);
		 */

		// Send a message with QoS 0
		MQTT::Message message;

		// QoS 0
		char buf[100];
		sprintf(buf, "I'm a jackrabbit\r\n");

		message.qos = MQTT::QOS0;
		message.retained = false;
		message.dup = false;
		message.payload = (void*)buf;
		message.payloadlen = strlen(buf)+1;

		/*
		rc = client.publish(topicGraphTemp, message);
		 */
		yieldAuto(&client);

		//les events queues
		event_queue.call_every(250, callback(yieldAuto, &client));
		event_queue.call_every(2000, callback(every_Seconds, &client));
		event_queue.call_every(500, test);

		// start events queues (fonction bloquante)
		event_queue.dispatch_forever();

	}

