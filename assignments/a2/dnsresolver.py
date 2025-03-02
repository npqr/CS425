import dns.message
import dns.query
import dns.rdatatype
import dns.resolver
import time

# Root DNS servers used to start the iterative resolution process
ROOT_SERVERS = {
    "198.41.0.4": "Root (a.root-servers.net)",
    "199.9.14.201": "Root (b.root-servers.net)",
    "192.33.4.12": "Root (c.root-servers.net)",
    "199.7.91.13": "Root (d.root-servers.net)",
    "192.203.230.10": "Root (e.root-servers.net)"
}

TIMEOUT = 3  # Timeout in seconds for each DNS query attempt

def send_dns_query(server, domain):
    """ 
    Sends a DNS query to the given server for an A record of the specified domain.
    Returns the response if successful, otherwise returns None.
    """
    try:
        query = dns.message.make_query(domain, dns.rdatatype.A)  # Construct the DNS query

        response = dns.query.udp(query, server, timeout=TIMEOUT)
        return response 

    except Exception:
        if((Exception) == dns.exception.Timeout):
            print(f"[ERROR] Query to {server} timed out")
        elif((Exception) == dns.resolver.NoAnswer):
            print(f"[ERROR] No answer found for {domain} on {server}")

        return None  # If an error occurs (timeout, unreachable server, etc.), return None

def extract_next_nameservers(response):
    """ 
    Extracts NS records from the authority section and resolves their IPs.
    Returns a list of IPs of the next authoritative nameservers.
    """
    ns_ips = []  # List to store resolved nameserver IPs
    ns_names = []  # List to store nameserver domain names

    # Loop through the authority section to extract NS records
    for rrset in response.authority:
        if rrset.rdtype == dns.rdatatype.NS:
            for rr in rrset:
                ns_name = rr.to_text().strip()
                ns_names.append(ns_name)
                print(f"Extracted NS hostname: {ns_name}")

    '''
    # Resolving IPs using contents of additional section

    additional_ips = {}

    # Extract A records from the additional section
    for rrset in response.additional:
        if rrset.rdtype == dns.rdatatype.A: 
            for rr in rrset:
                additional_ips[rrset.name.to_text().strip()] = rr.address
                ns_ips.append(rr.address)
                print(f"Resolved {rrset.name} -> {rr.address} [Additional Section]")
    '''

    # Resolve remaining NS names to IPs (if not found in additional section)
    for ns_name in ns_names:
        '''
        # continued resolution of IPs using additional section

        curr_ns_ip = None
        
        if ns_name in additional_ips:
            continue  # Skip if already resolved


        for root_ip in ROOT_SERVERS:  # Try each root server
            response = send_dns_query(root_ip, ns_name)

            if response and response.answer:
                for rrset in response.answer:
                    if rrset.rdtype == dns.rdatatype.A:
                        for rr in rrset:
                            curr_ns_ip = rr.address
                            ns_ips.append(rr.address)
                            print(f"Resolved {ns_name} -> {rr.address} [Root Server Query]")
                            break  # Stop after the first success
        

        if(not curr_ns_ip):
            curr_ns_ip = dns.resolver.resolve(ns_name, "A")[0].address
            ns_ips.append(curr_ns_ip)
            print(f"Resolved {ns_name} -> {curr_ns_ip}")
        '''
        ns_ip = dns.resolver.resolve(ns_name, "A")[0].address

        if(ns_ip): # Check if IP is resolved
            print(f"Resolved {ns_name} -> {ns_ip}")
            ns_ips.append(ns_ip)
        else:
            print(f"[ERROR] Failed to resolve {ns_name}")

    return ns_ips  # Return list of resolved nameserver IPs

def iterative_dns_lookup(domain):
    """ 
    Performs an iterative DNS resolution starting from root servers.
    It queries root servers, then TLD servers, then authoritative servers,
    following the hierarchy until an answer is found or resolution fails.
    """
    print(f"[Iterative DNS Lookup] Resolving {domain}")

    next_ns_list = list(ROOT_SERVERS.keys())  # Start with the root server IPs
    stage = "ROOT"  # Track resolution stage (ROOT, TLD, AUTH)

    while next_ns_list:
        ns_ip = next_ns_list[0]  # Pick the first available nameserver to query
        print(f"[DEBUG] Querying {stage} server ({ns_ip})")
        response = send_dns_query(ns_ip, domain)
        
        if response: #checks if response is not NONE            
            if response.answer:

                # modified the initial code which checked only for 
                # response.answer[0][0], assuming it to be an A record

                # Iterate through the answer section to find the A record
                for rrset in response.answer:
                    if rrset.rdtype == dns.rdatatype.A:
                        for rr in rrset:
                            print(f"[SUCCESS] {domain} -> {rr.address}")
                            return
                
                # If no A record found, print the CNAME record
                for rrset in response.answer:
                    if rrset.rdtype == dns.rdatatype.CNAME:
                        print(f"[SUCCESS] {domain} -> {rrset[0].target}")
                        return

                # No A or CNAME records found
                print(f"[ERROR] No A or CNAME records found for {domain} on {ns_ip}")
                print("Trying another nameserver...")
                next_ns_list.pop(0)
                continue
            
            # If no answer, extract the next set of nameservers
            next_ns_list = extract_next_nameservers(response)

            if(stage == "ROOT"):    # Move to the next stage of resolution
                stage = "TLD"
            elif(stage == "TLD"):
                stage = "AUTH"

        else:
            if(not next_ns_list):   # Check if there are no more nameservers to query
                print(f"[ERROR] Query failed for {stage} {ns_ip}") # Print error message if query fails
                print("[ERROR] Resolution failed.")  # Final failure message if no nameservers respond
                return  # Stop resolution if a query fails
            else:
                print(f"[ERROR] Query failed for {stage} {ns_ip} - Moving to next NS") # Retry with the next nameserver
                next_ns_list.pop(0) # Remove the failed nameserver from the list

    print("[ERROR] Resolution failed.")  # Final failure message if no nameservers respond

def recursive_dns_lookup(domain):
    """ 
    Performs recursive DNS resolution using the system's default resolver.
    This approach relies on a resolver (like Google DNS or a local ISP resolver)
    to fetch the result recursively.
    """
    print(f"[Recursive DNS Lookup] Resolving {domain}")
    try:
        answer = dns.resolver.resolve(domain, "NS") # Resolve NS records for the domain
        for rdata in answer:
            print(f"[SUCCESS] {domain} -> {rdata}")

        answer = dns.resolver.resolve(domain, "A")
        for rdata in answer:
            print(f"[SUCCESS] {domain} -> {rdata}")

    except Exception as e:
        print(f"[ERROR] Recursive lookup failed: {e}")  # Handle resolution failure

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 3 or sys.argv[1] not in {"iterative", "recursive"}:
        print("Usage: python3 dns_server.py <iterative|recursive> <domain>")
        sys.exit(1)

    mode = sys.argv[1]  # Get mode (iterative or recursive)
    domain = sys.argv[2]  # Get domain to resolve
    start_time = time.time()  # Record start time
    
    # Execute the selected DNS resolution mode
    if mode == "iterative":
        iterative_dns_lookup(domain)
    else:
        recursive_dns_lookup(domain)
    
    print(f"Time taken: {time.time() - start_time:.3f} seconds")  # Print execution time
