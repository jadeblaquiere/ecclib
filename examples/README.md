# ecclib examples

These examples are intended to illustrate usage of the basic ECC primatives. 
They exercise the major features of the C API for curves and point. (Note, there
are also interoperable equivalents using the [python API](../python/examples/)
and the [Go API](../ecgo/cmd/)

# Tutorial

NOTE : the example programs are not built by default. If you want to build the
examples you need to ensure that you passed **--enable-examples** as a command line
option to the **configure** script. The examples are intended to be just that,
examples. While the example programs are intended to demonstrate a practical,
working and complete cryptosystem, they haven't been audited and implement
a rudimentary interface which leaves several details (e.g. authenticity of keys,
securing private keys) up to the user.

## Creating Keys

1. Create a public and private key pair

    As with most cryptography examples we have the situation where Alice wishes
    to send a message to Bob. In order for this to happen, Bob must have created
    a private key and then derive a public key (which he provides Alice). It is
    not necessary that Alice has keys to send a message to Bob. Bob also wishes
    to ensure his messages are highly secure so he selects a curve with a large
    prime field.
    
    ```
    ./ecdh_gen --list-curves
    ./ecdh_gen --curve=Curve41417 > alice.privkey
    ./ecdh_pub --file alice.privkey > alice.pubkey
    ./ecdh_gen --curve=Curve41417 > bob.privkey
    ./ecdh_pub --file bob.privkey > bob.pubkey
    cat alice.privkey
    cat alice.pubkey
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
    echo "Hello, Bob!" | ./ecdh_enc --pubkey=bob.pubkey > a2b.ctxt
    cat a2b.ctxt
    ```
    
    Note that the encoding uses a random ephemeral private key and nonce and
    passes the corresponding public key point and nonce in the message so
    running the encryption command again with the same message input will
    generate a different ciphertext.

1. Sign the Message

    Alice wishes that Bob can verify the authenticity of the message by signing
    the message using the Elliptic Curve Digital Signature Algrithm (ECDSA). 
    Alice uses her private key to sign the ciphertext.

    ```
    cat a2b.ctxt | ./ecdsa_sign --privkey=alice.privkey > a2b_sign.ctxt
    cat a2b_sign.ctxt
    ```

1. Validate the Message Origin

    Upon reciept of the message Bob first validates that the message was
    signed with Alice's private key using Alice's public key.

    ```
    cat a2b_sign.ctxt | ./ecdsa_verify --pubkey=alice.pubkey > a2b_copy.ctxt
    cat a2b_copy.ctxt
    ```

1. Decode the message

    Bob can use his private key to decode the message. Simple enough.

    ```
    cat a2b.ctxt | ./ecdh_dec --privkey=bob.privkey
    ```
