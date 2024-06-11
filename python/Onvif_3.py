from zeep import Client, Settings, Transport
from requests.auth import HTTPDigestAuth
import requests
import time
import re

# Path to your local WSDL file
wsdl_path = 'f/events.wsdl'

def modify_wsdl(wsdl_path, old_ip, old_port, new_ip, new_port):
    with open(wsdl_path, 'r', encoding='utf-8') as file:
        wsdl = file.read()

    # Replace the old IP and port with the new ones
    wsdl = re.sub(f'{old_ip}:{old_port}', f'{new_ip}:{new_port}', wsdl)
    # return wsdl
    with open(wsdl_path, 'w', encoding='utf-8') as file:
        file.write(wsdl)

# modify_wsdl(wsdl_path, '192.168.0.51', '8888', '10.0.0.26', '8000')

# Create a session with Digest Authentication
session = requests.Session()
session.auth = HTTPDigestAuth('admin', 'anna.landa85')  # Use your camera credentials
transport = Transport(session=session)
settings = Settings(strict=False, xml_huge_tree=True)

# Load the WSDL
client = Client(wsdl=wsdl_path, transport=transport, settings=settings)

# Print available services and ports to verify correct names
print("Available services and ports:")
for service in client.wsdl.services.values():
    print(f"Service: {service.name}")
    for port in service.ports.values():
        print(f"  Port: {port.name}")

# Bind to the EventService (adjust names as needed)
try:
    event_service = client.bind('EventService', 'EventPortType')  # Adjust these names based on previous script output
    print("EventService bound successfully.")
except Exception as e:
    print(f"Error binding to EventService: {e}")
    raise

# List available operations for EventService
print("Available operations for Event Service:")
for operation in event_service._binding._operations.values():
    print(f"  Operation: {operation.name}")


print("\n\n\n\n\nEvent Service object:")
#  print ou the event_service object and everything it contains
for i in dir(event_service):
    print(i)
# print(event_service)
# # Create PullPointSubscription
# try:
#     pullpoint_subscription = event_service.CreatePullPointSubscription()
#     subscription_reference = pullpoint_subscription.SubscriptionReference.Address
#     print(f"Subscription Reference: {subscription_reference}")
# except Exception as e:
#     print(f"Error creating PullPointSubscription: {e}")
#     raise

# # Bind to the PullPointSubscription service
# try:
#     pullpoint_service = client.bind('PullPointSubscription', 'PullPointSubscriptionBinding')  # Adjust these names based on previous script output
#     print("PullPointSubscription bound successfully.")
# except Exception as e:
#     print(f"Error binding to PullPointSubscription: {e}")
#     raise

# # Function to handle the received events
# def event_callback(event):
#     print(f"Event: {event}")

# # Pull messages from the subscription
# while True:
#     try:
#         pull_messages = pullpoint_service.PullMessages({
#             'Timeout': 'PT10S',
#             'MessageLimit': 10
#         })

#         for msg in pull_messages.NotificationMessage:
#             event_callback(msg)

#         time.sleep(10)  # Wait for 10 seconds before pulling the next messages
#     except Exception as e:
#         print(f"Error during pull messages: {e}")
#         break
