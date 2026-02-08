## EKU — _Enhanced Key Usage_

### What EKU is (core idea)

> **EKU defines _what a certificate is allowed to be used for_.**

A certificate is not just “trusted” or “untrusted” — it has **constraints**.

EKUs answer:

> “This certificate may be used for **X**, but not **Y**.”

---

### Common EKU examples

|EKU|Meaning|
|---|---|
|Code Signing|Sign executables|
|Server Authentication|TLS servers|
|Client Authentication|TLS clients|
|Driver Signing|Kernel drivers|
|Windows System Component Verification|Core Windows binaries|

---

### EKU in Protected Processes / PPL

Microsoft introduced **special EKUs** that say:

> “This certificate is allowed to create protected or PPL processes.”

Examples (from the book):

- `1.3.6.1.4.1.311.10.3.22`
    
- `1.3.6.4.1.311.10.3.20`
    

These EKUs:

- Are checked by **Code Integrity**
    
- Are mapped to **PPL signer levels**
    
- Cannot be obtained by normal developers
    

---

### Why EKU matters for PPL

Even if malware:

- Is signed
    
- Has a valid cert
    

❌ It **cannot**:

- Claim `PsProtectedSignerAntimalware`
    
- Claim `PsProtectedSignerWinTcb`
    

Unless the cert has:

- Correct EKU
    
- Correct issuer
    
- Correct Microsoft trust mapping