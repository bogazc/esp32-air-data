import paho.mqtt.client as mqtt
import datetime
import sqlite3
import json

MQTT_BROKER = "mqtt-broker"
MQTT_TOPIC = "sensors/air_quality"
DB_FILE = "air_quality.db"

def setup_database():
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    cursor.execute('''
    CREATE TABLE IF NOT EXISTS measurements (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME,
                    station_id TEXT,
                    temperature REAL,
                    pressure REAL,
                    altidute REAL,
                    pm1_0 INTEGER,
                    pm2_5 INTEGER,
                    pm10_0 INTEGER
                    )
    ''')
    conn.commit()
    conn.close()

def send_to_database(data):
    try:
        conn = sqlite3.connect(DB_FILE)
        cursor = conn.cursor()
        now = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')

        query = '''
                    INSERT INTO measurements 
                    (timestamp, station_id, temperature, pressure, altidute, pm1_0, pm2_5, pm10_0)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?)
                '''
                
        values = (
            now,
            data.get('client'),
            data.get('temp'),
            data.get('press'),
            data.get('altidute'), 
            data.get('pm10'),
            data.get('pm25'),
            data.get('pm100')
        )

        cursor.execute(query, values)
        conn.commit()
        conn.close()
        print(f"[{now}] Data saved successfully.")
    except Exception as e:
        print(f"Error writing to database: {e}")

def on_connect(client, userdata, flags, rc):
    print(f"Connected to broker. Subscribing to {MQTT_TOPIC}")
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        send_to_database(payload)
    except Exception as e:
        print(f"Error processing message: {e}")

if __name__ == "__main__":
    setup_database()
    
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(MQTT_BROKER, 1883, 60)
    client.loop_forever()