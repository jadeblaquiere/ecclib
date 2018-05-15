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
	"flag"
	// "fmt"
	"github.com/jadeblaquiere/ecclib/ecgo"
	"io/ioutil"
	"os"
)

func main() {
	var input []byte
	var err error

	// HANDLE FLAGS

	filepath := flag.String("file", "", "private key input file path (default: read from stdin)")
	flag.Parse()

	// READ INPUT DATA, UNPACK AND DECODE

	if len(*filepath) == 0 {
		input, err = ioutil.ReadAll(os.Stdin)
		shared.ExitOnError(err, "Error reading input from stdin")
	} else {
		input, err = ioutil.ReadFile(*filepath)
		shared.ExitOnError(err, "Error reading input from file")
	}

	der, err := shared.ReadB64Wrapped("ECDH PRIVATE KEY", input)

	privkey, cv, err := shared.ECDHPrivkeyFromDER(der)
	shared.ExitOnError(err, "<Error>: error parsing PRIVATE key\n")

	// CALCULATE PUBLIC KEY POINT BASED ON PRIVATE KEY AND CURVE GENERATOR POINT

	G := ecgo.NewPointGenerator(cv)
	Pubkey := ecgo.NewPointNeutral(cv)

	Pubkey.Mul(G, privkey)

	// EXPORT PUBLIC KEY, CURVE

	der, err = shared.ECDHPubkeyToDER(Pubkey, cv)

	shared.WriteB64Wrapped("ECDH PUBLIC KEY", der)
}
