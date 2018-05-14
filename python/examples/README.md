# ecclib examples

These examples are intended to illustrate usage of the basic ECC primatives. 
They exercise the major features of the Python API for curves and point. There
are also [interoperable equivalents using the C API](../../examples/)


# Tutorial

NOTE : The examples are intended to be just that, examples. While the example
programs are intended to demonstrate a practical, working and complete
cryptosystem, they haven't been audited and implement a rudimentary interface
which leaves several details (e.g. authenticity of keys, securing private keys)
up to the user.

## Creating Keys

1. Create a public and private key pair

    As with most cryptography examples we have the situation where Alice wishes
    to send a message to Bob. In order for this to happen, Bob must have created
    a private key and then derive a public key (which he provides Alice). It is
    not necessary that Alice has keys to send a message to Bob. Bob also wishes
    to ensure his messages are highly secure so he selects a curve with a large
    prime field.
    
    ```
    python3 ecdh_gen.py --list_curves
    python3 ecdh_gen.py --curve=Curve41417 > bob.privkey
    python3 ecdh_pub.py --file bob.privkey > bob.pubkey
    cat bob.privkey
    cat bob.pubkey
    ```
    
    The keys as encoded include the key values and the curve parameters.

## Sending a message

1. Encode the Message

    Alice encodes a message to Bob. Once the message is encoded it can be
    delivered by any means to Bob. The contents of the message cannot be
    decoded without access to the private key.

    ```
    echo "Hello, Bob!" | python3 ecdh_enc.py --pubkey=bob.pubkey > a2b.ctxt
    cat a2b.ctxt
    ```
    
    Note that the encoding uses a random ephemeral private key and nonce and
    passes the corresponding public key point and nonce in the message so
    running the encryption command again with the same message input will
    generate a different ciphertext.

1. Decode the message

    Bob can use his private key to decode the message. Simple enough.
    
    ```
    cat a2b.ctxt | python3 ecdh_dec.py --privkey=bob.privkey
    ```
