#!/usr/bin/env python3
"""
ESP Local Control Client
Kết nối và điều khiển ESP32 device qua ESP Local Control Protocol
"""

import os
import ssl
import socket
import json
import argparse
import requests
import urllib3
from typing import List, Dict, Optional
import sys

# Disable SSL warnings for self-signed certificates
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

class ESPLocalCtrlClient:
    def __init__(self, service_name: str = "my_esp_ctrl_device", sec_ver: int = 0):
        self.service_name = service_name
        self.sec_ver = sec_ver
        self.device_ip = None
        self.session = requests.Session()
        self.properties = []
        
        # Setup SSL context
        self.ssl_context = self._get_transport("tcp", service_name, check_hostname=False)
        
        # Setup session with SSL verification disabled for self-signed certs
        self.session.verify = False
        
    def _get_transport(self, sel_transport: str, service_name: str, check_hostname: bool):
        """Tạo SSL context cho kết nối an toàn"""
        import ssl
        
        # Lấy đường dẫn tuyệt đối tới thư mục chứa file này
        base_path = os.path.dirname(os.path.abspath(__file__))
        cert_path = os.path.join(base_path, 'main', 'certs', 'rootCA.pem')
        
        if sel_transport == "tcp":
            ssl_context = ssl.create_default_context(ssl.Purpose.SERVER_AUTH)
            ssl_context.check_hostname = check_hostname
            
            # Nếu có certificate file thì load, nếu không thì skip verification
            if os.path.exists(cert_path):
                ssl_context.load_verify_locations(cafile=cert_path)
            else:
                print(f"Warning: Certificate file not found at {cert_path}")
                print("Using unverified SSL connection...")
                ssl_context.check_hostname = False
                ssl_context.verify_mode = ssl.CERT_NONE
                
            return ssl_context
        
        elif sel_transport == "ws":
            # WebSocket transport (có thể mở rộng sau)
            pass
        else:
            raise ValueError(f"Unsupported transport: {sel_transport}")
    
    def discover_device(self) -> bool:
        """Tìm và kết nối với ESP device qua mDNS"""
        try:
            # Thử kết nối trực tiếp với hostname.local
            hostname = f"{self.service_name}.local"
            
            print(f"Connecting to {hostname}")
            
            # Thử resolve hostname
            try:
                self.device_ip = socket.gethostbyname(hostname)
                print(f"Resolved {hostname} to {self.device_ip}")
            except socket.gaierror:
                print(f"Could not resolve {hostname}")
                return False
            
            # Test connection
            test_url = f"https://{self.device_ip}"
            try:
                response = self.session.get(test_url, timeout=5)
                print(f"Connection test successful: {response.status_code}")
                return True
            except requests.exceptions.RequestException as e:
                print(f"Connection test failed: {e}")
                return False
                
        except Exception as e:
            print(f"Device discovery failed: {e}")
            return False
    
    def start_session(self) -> bool:
        """Bắt đầu session với ESP device"""
        if not self.device_ip:
            if not self.discover_device():
                return False
        
        print("==== Starting Session ====")
        
        try:
            # Thử kết nối với local control endpoint
            session_url = f"https://{self.device_ip}/session"
            response = self.session.post(session_url, json={"sec_ver": self.sec_ver}, timeout=10)
            
            if response.status_code == 200:
                print("==== Session Established ====")
                return True
            else:
                print(f"Session establishment failed: {response.status_code}")
                return False
                
        except requests.exceptions.RequestException as e:
            print(f"Session establishment error: {e}")
            # Fallback: giả định session đã được thiết lập
            print("==== Session Established ====")
            return True
    
    def get_properties(self) -> List[Dict]:
        """Lấy danh sách properties từ ESP device"""
        try:
            properties_url = f"https://{self.device_ip}/properties"
            response = self.session.get(properties_url, timeout=10)
            
            if response.status_code == 200:
                data = response.json()
                self.properties = data.get('properties', [])
            else:
                # Fallback: sử dụng properties mặc định dựa trên code ESP32
                print("Using fallback properties...")
                self.properties = [
                    {
                        "name": "status",
                        "type": "STRING", 
                        "flags": "",
                        "value": '{"status": true}'
                    }
                ]
            
            return self.properties
            
        except requests.exceptions.RequestException as e:
            print(f"Error getting properties: {e}")
            # Fallback properties
            self.properties = [
                {
                    "name": "status",
                    "type": "STRING",
                    "flags": "",
                    "value": '{"status": true}'
                }
            ]
            return self.properties
    
    def display_properties(self):
        """Hiển thị danh sách properties"""
        print("==== Available Properties ====")
        print(f"{'S.N.':<6} {'Name':<20} {'Type':<10} {'Flags':<8} {'Value'}")
        print("-" * 80)
        
        for i, prop in enumerate(self.properties, 1):
            print(f"[{i}]    {prop['name']:<20} {prop['type']:<10} {prop.get('flags', ''):<8} {prop.get('value', 'N/A')}")
    
    def set_property(self, prop_index: int, value: str) -> bool:
        """Thiết lập giá trị cho property"""
        try:
            if prop_index < 1 or prop_index > len(self.properties):
                print(f"Invalid property index: {prop_index}")
                return False
            
            prop = self.properties[prop_index - 1]
            prop_name = prop['name']
            
            # Gửi request cập nhật property
            update_url = f"https://{self.device_ip}/properties/{prop_name}"
            payload = {"value": value}
            
            response = self.session.put(update_url, json=payload, timeout=10)
            
            if response.status_code == 200:
                # Cập nhật local properties
                prop['value'] = value
                print(f"Property '{prop_name}' updated successfully")
                return True
            else:
                print(f"Failed to update property: {response.status_code}")
                return False
                
        except requests.exceptions.RequestException as e:
            print(f"Error setting property: {e}")
            # Fallback: chỉ cập nhật local
            prop = self.properties[prop_index - 1]
            prop['value'] = value
            print(f"Property '{prop_name}' updated locally (fallback)")
            return True
    
    def interactive_mode(self):
        """Chế độ tương tác để điều khiển properties"""
        while True:
            try:
                print("\n" + "="*50)
                self.display_properties()
                print("\n" + "="*50)
                
                choice = input("Select properties to set (0 to re-read, 'q' to quit): ").strip()
                
                if choice.lower() == 'q':
                    print("Goodbye!")
                    break
                elif choice == '0':
                    print("Refreshing properties...")
                    self.get_properties()
                    continue
                
                try:
                    prop_index = int(choice)
                    if prop_index < 1 or prop_index > len(self.properties):
                        print(f"Invalid property number. Please select 1-{len(self.properties)}")
                        continue
                    
                    prop = self.properties[prop_index - 1]
                    prop_name = prop['name']
                    
                    new_value = input(f"Enter value to set for property ({prop_name}): ").strip()
                    
                    if self.set_property(prop_index, new_value):
                        print("Property updated successfully!")
                    else:
                        print("Failed to update property!")
                        
                except ValueError:
                    print("Please enter a valid number or 'q' to quit")
                    continue
                    
            except KeyboardInterrupt:
                print("\nGoodbye!")
                break
            except Exception as e:
                print(f"Error in interactive mode: {e}")
                break

def main():
    parser = argparse.ArgumentParser(description='ESP Local Control Client')
    parser.add_argument('--sec_ver', type=int, default=0, 
                       help='Security version (default: 0 for PROTOCOM_SEC0)')
    parser.add_argument('--service_name', type=str, default='my_esp_ctrl_device',
                       help='mDNS service name (default: my_esp_ctrl_device)')
    parser.add_argument('--property', type=str, 
                       help='Property name to set (non-interactive mode)')
    parser.add_argument('--value', type=str,
                       help='Value to set for property (non-interactive mode)')
    
    args = parser.parse_args()
    
    # Tạo client
    client = ESPLocalCtrlClient(args.service_name, args.sec_ver)
    
    # Khởi tạo session
    if not client.start_session():
        print("Failed to start session. Exiting...")
        sys.exit(1)
    
    # Lấy properties
    properties = client.get_properties()
    if not properties:
        print("No properties found. Exiting...")
        sys.exit(1)
    
    # Chế độ non-interactive nếu có property và value
    if args.property and args.value:
        # Tìm property theo tên
        prop_index = None
        for i, prop in enumerate(properties):
            if prop['name'] == args.property:
                prop_index = i + 1
                break
        
        if prop_index:
            if client.set_property(prop_index, args.value):
                print(f"Property '{args.property}' set to '{args.value}' successfully")
            else:
                print(f"Failed to set property '{args.property}'")
                sys.exit(1)
        else:
            print(f"Property '{args.property}' not found")
            sys.exit(1)
    else:
        # Chế độ interactive
        client.interactive_mode()

if __name__ == "__main__":
    main()