# CS425 A2: DNS Resolution - Iterative and Recursive Lookup

## How to Run

### Prerequisites

It is recommended to run the code on a Linux machine and to run in a Python virtual environment.

Steps for creating a virtual environment:

Install `virtualenv`.
```bash
pip3 install virtualenv
```
Create a virtual environment.
```bash
python3 -m venv dnsenv
```
Activate the virtual environment.
```bash
source dnsenv/bin/activate
```
Install the required packages.
```bash
pip install dnspython
```

### Running the Code

After creating the virtual environment, follow the steps below to run the code:

```bash
python3 dnsresolver.py <iterative|recursive> <domain_name>
```
where `<iterative|recursive>` is the type of DNS resolver to be used and `<domain_name>` is the domain name to be resolved.

## Design Decisions

- Used the default dns resolver for resolving the IPs of name servers in the `extract_next_nameservers` function. Rationale : We attempted to resolve the IPs of name servers using the additional and answer sections, but it fails (returns incorrect IPs in some cases, e.g. for kendra.cse.iitk.ac.in, the IP of ns1.iitk.ac.in is resolved to 202.3.77.171 instead of 172.31.1.74)
- Used only A records and not AAAA records for resolving the IPs of name servers. Rationale : We are not handling IPv6 addresses in this assignment.
- The `answer[0][0]` may not always be an A record. Thus, we find the first A record in the answer section and use it to resolve the IP of the name server. If no A record, we return the CNAME record. If neither, we return an error. 
- If the resolution fails on a name server, we try the next name server in the list of name servers. If all name servers fail, we return an error.
- Added explicit errors for Timeouts and NoAnswer.

## Team Members

- Khushi Gupta (220531)
- Nevish Pathe (220757)

## Sources

- [dnspython documentation](https://dnspython.readthedocs.io/en/stable/)

## Declaration

We declare that we did not indulge in plagiarism and the work submitted is our own.