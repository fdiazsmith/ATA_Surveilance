import requests
from zeep import Client, Settings, Transport
from requests.auth import HTTPDigestAuth
import os
import time

import re

def modify_wsdl(wsdl_path, old_ip, old_port, new_ip, new_port):
    with open(wsdl_path, 'r', encoding='utf-8') as file:
        wsdl = file.read()

    # Replace the old IP and port with the new ones
    wsdl = re.sub(f'{old_ip}:{old_port}', f'{new_ip}:{new_port}', wsdl)
    # return wsdl
    with open(wsdl_path, 'w', encoding='utf-8') as file:
        file.write(wsdl)

# Path to the local WSDL files
wsdl_base_path = 'wsdl/'
# Modify the WSDL files
device_wsdl = os.path.join(wsdl_base_path, 'devicemgmt.wsdl')
events_wsdl = os.path.join(wsdl_base_path, 'events.wsdl')
bw2_wsdl = os.path.join(wsdl_base_path, 'bw-2.wsdl')

modify_wsdl(device_wsdl, '192.168.0.51', '8888', '10.0.0.26', '8000')
modify_wsdl(events_wsdl, '192.168.0.51', '8888', '10.0.0.26', '8000')
modify_wsdl(bw2_wsdl, '192.168.0.51', '8888', '10.0.0.26', '8000')


# Create a Zeep client with digest authentication for the event service
session = requests.Session()
session.auth = HTTPDigestAuth('admin', 'anna.landa85')
transport = Transport(session=session)
settings = Settings(strict=False, xml_huge_tree=True)
client = Client(events_wsdl, transport=transport, settings=settings)

# List available operations
print("Available operations:")
for service in client.wsdl.services.values():
    print(f"Service: {service.name}")
    for port in service.ports.values():
        print(f"  Port: {port.name}")
        for operation in port.binding._operations.values():
            print(f"    Operation: {operation.name}")

# Interact with the event service using available operations
# event_service = client.bind('EventService', 'EventPort')
# event_service = client.bind('EventService', 'EventPortType')
event_service = client.bind('EventService', 'PullPointSubscription')

# Create PullPointSubscription
pullpoint = event_service.CreatePullPointSubscription()
print(f"Subscription Reference: {pullpoint.SubscriptionReference.Address}")

# Pull messages from the subscription
while True:
    try:
        pull_messages = event_service.PullMessages({
            'SubscriptionReference': pullpoint.SubscriptionReference,
            'MessageLimit': 10,
            'Timeout': 'PT10S'
        })

        for msg in pull_messages.NotificationMessage:
            print(f"Event: {msg}")

        time.sleep(10)  # Wait for 10 seconds before pulling the next messages
    except Exception as e:
        print(f"Error during pull messages: {e}")
        break
