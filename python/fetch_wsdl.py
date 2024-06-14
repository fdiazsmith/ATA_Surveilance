import requests
from requests.auth import HTTPDigestAuth
from lxml import etree

# Camera connection details
camera_ip = "10.0.0.26"
camera_port = 8000
username = "admin"
password = "anna.landa85"

# SOAP Envelope to get the event properties (you may need to adjust the namespaces and request based on your camera's requirements)
soap_body = """<?xml version="1.0" encoding="UTF-8"?>
<s:Envelope xmlns:s="http://www.w3.org/2003/05/soap-envelope"
            xmlns:tev="http://www.onvif.org/ver10/events/wsdl">
  <s:Body>
    <tev:GetEventProperties/>
  </s:Body>
</s:Envelope>"""

# URL of the event service
url = f"http://{camera_ip}:{camera_port}/onvif/event_service"

# Headers for the SOAP request
headers = {
    'Content-Type': 'application/soap+xml; charset=utf-8',
}

# Send the SOAP request
response = requests.post(url, data=soap_body, headers=headers, auth=HTTPDigestAuth(username, password))

# Check if the request was successful
if response.status_code == 200:
    # Save the response content to a file (you might need to adjust the parsing to get the exact WSDL content)
    with open('event_service_response.xml', 'wb') as file:
        file.write(response.content)
    print("WSDL response saved successfully.")
else:
    print(f"Failed to retrieve WSDL. Status code: {response.status_code}")
    print(response.text)

# If you need to parse and save the exact WSDL from the response, you can use lxml to extract it.
root = etree.fromstring(response.content)
wsdl_element = root.find('.//{http://schemas.xmlsoap.org/wsdl/}definitions')
if wsdl_element is not None:
    with open('event_service.wsdl', 'wb') as wsdl_file:
        wsdl_file.write(etree.tostring(wsdl_element, pretty_print=True))
    print("WSDL extracted and saved successfully.")
else:
    print("Failed to find WSDL definitions in the response.")
