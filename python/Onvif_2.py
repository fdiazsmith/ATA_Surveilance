from onvif import ONVIFCamera
from requests.auth import HTTPDigestAuth
import requests

# Camera connection details
camera_ip = "10.0.0.26"
camera_port = 8000
username = "admin"
password = "anna.landa85"


def validate_onvif_service(ip, port, user, pwd):
    try:
        camera = ONVIFCamera(ip, port, user, pwd)
        # Create the device management service
        devicemgmt = camera.create_devicemgmt_service()
        
        # Get device capabilities
        capabilities = devicemgmt.GetCapabilities({'Category': 'All'})
        print("Device capabilities retrieved successfully.")
        return capabilities
    except Exception as e:
        print(f"Failed to connect to ONVIF service: {e}")
        return None

capabilities = validate_onvif_service(camera_ip, camera_port, username, password)
if capabilities:
    print("ONVIF service is active and responding.")
    # print each of the capabilities
    for capability in capabilities:
        print(f"Capability: {capability}")
else:
    print("ONVIF service is not responding.")

def list_services(ip, port, user, pwd):
    try:
        camera = ONVIFCamera(ip, port, user, pwd)
        # Create the device management service
        devicemgmt = camera.create_devicemgmt_service()
        
        # List services
        services = devicemgmt.GetServices(False)
        print("Available services:")
        for service in services:
            print(f"Namespace: {service['Namespace']}")
            print(f"XAddr: {service['XAddr']}")
            print(f"Version: {service['Version']}\n")
        return services
    except Exception as e:
        print(f"Failed to list ONVIF services: {e}")
        return None

services = list_services(camera_ip, camera_port, username, password)
if services:
    print("ONVIF services listed successfully.")
else:
    print("Failed to list ONVIF services.")
