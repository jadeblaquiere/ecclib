//BSD 3-Clause License
//
//Copyright (c) 2018, jadeblaquiere
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//
//* Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//* Neither the name of the copyright holder nor the names of its
//  contributors may be used to endorse or promote products derived from
//  this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package main

import (
	"../shared"
	//"crypto/rand"
	//"crypto/sha256"
	"flag"
	// "fmt"
	"github.com/jadeblaquiere/ecclib/ecgo"
	//"golang.org/x/crypto/salsa20"
	"io/ioutil"
	"os"
)

func main() {
	var input []byte
	var err error

	// HANDLE FLAGS

	filepath := flag.String("file", "", "plaintext input")
	keyfile := flag.String("pubkey", "", "public key input file path (default: read from stdin)")
	flag.Parse()

	// READ INPUT DATA, UNPACK AND DECODE

	if len(*keyfile) == 0 {
		shared.ExitWMessage(1, "Error: -pubkey option is mandatory")
	}
	keyb64, err := ioutil.ReadFile(*keyfile)
	shared.ExitOnError(err, "Error reading key from file")

	der, err := shared.ReadB64Wrapped("ECDH PUBLIC KEY", keyb64)
	shared.ExitOnError(err, "Unable to decode base64 public key data")

	Pubkey, cv, err := shared.ECDHPubkeyFromDER(der)
	shared.ExitOnError(err, "<Error>: error parsing PUBLIC key\n")

	if len(*filepath) == 0 {
		input, err = ioutil.ReadAll(os.Stdin)
		shared.ExitOnError(err, "Error reading input from stdin")
	} else {
		input, err = ioutil.ReadFile(*filepath)
		shared.ExitOnError(err, "Error reading input from file")
	}

	der, err = shared.ReadB64Wrapped("ECDH PUBLIC KEY", input)
	shared.ExitOnError(err, "Unable to decode base64 public key data")

	ptxt, cv2, err := shared.ECDHPubkeyFromDER(der)
	shared.ExitOnError(err, "<Error>: error parsing plaintext (PUBLIC key)\n")

	if cv.Cmp(cv2) != 0 {
		shared.ExitWMessage(1, "Error: pubkey and plaintext point must be from same curve")
	}

	// ENCRYPT USING PUBLIC KEY

	ctxt := ecgo.ECElgamalEncrypt(Pubkey, ptxt)

	// EXPORT CIPHERTEXT, CURVE AS MESSAGE

	der, err = shared.ECElgamalToDER(ctxt, cv)
	shared.ExitOnError(err, "error contructing DER format")

	shared.WriteB64Wrapped("ECELGAMAL ENCRYPTED MESSAGE", der)
}
