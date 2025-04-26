
Script.require('../cryptographyDebugger.js');

const generateRandomNumber = (max, min = 0) => Math.floor(Math.random() * (max - min + 1)) + min;

function runRSATest(bitLength) {
	// Preforms all functions relating to RSA cryptography
	debugLog(`---`);
	debugLog(`Starting RSA test for ${bitLength} length RSA key.`);

	// We first need to generate a RSA key pair 
	let keyPair = generateRSAKeyPair(bitLength);
	if (!keyPair.success) return debugLog(keyPair.message);
	keyPair = keyPair.message;

	// Next, we will test RSA signing and verification.
	// We generate two sets. One to sign and with signatures, and another not to sign.
	// The ones we do not sign we attempt to verify and hope to receive an error.
	const MESSAGES_TO_SIGN = generateNewMessages(3, 200);
	const MESSAGES_NOT_TO_SIGN = generateNewMessages(3, 200);
	const MESSAGES_TO_ENCRYPT = generateNewMessages(3, 5);

	// Generate the key signatures.
	let messageSignatures = generateSignaturesFromMessages(MESSAGES_TO_SIGN, keyPair.private);
	if (!messageSignatures.success) return debugLog(messageSignatures.message);
	messageSignatures = messageSignatures.message;

	// Test the key signatures.
	debugLog(`Starting signature validation test...`);
	let ALL_SIGNATURES_PASSED = testSignaturesFromMessages(MESSAGES_TO_SIGN, messageSignatures, keyPair.public);
	if (!ALL_SIGNATURES_PASSED.success) return debugLog(ALL_SIGNATURES_PASSED.message);
	debugLog(`All signatures verified!`);


	// Here we are testing an invalid message and signature combination. The expected response here is 'false'.
	// 'false' values are considered success in this context.  
	debugLog(`Starting signature failure validation test. There should be ${MESSAGES_NOT_TO_SIGN.length} failures, these are expected.`);
	const ALL_INVALID_SIGNATURES_PASSED = testSignaturesFromMessages(MESSAGES_NOT_TO_SIGN, messageSignatures, keyPair.public, true);
	if (ALL_INVALID_SIGNATURES_PASSED.success) return debugLog(`Invalid RSA signatures somehow managed to be valid.`);
	debugLog(`All invalid signatures tested!`)

	// Next we test the encryption and decryption of RSA messages.
	// There is an upper limit to the length we can encrypt messages, so we use smaller values here.
	// First we encrypt.
	debugLog(`Encrypting ${MESSAGES_TO_ENCRYPT.length} messages.`);
	let encryptedMessages = encryptRSAMessages(MESSAGES_TO_ENCRYPT, keyPair.public);
	if (!encryptedMessages.success) return debugLog(encryptedMessages.message);
	encryptedMessages = encryptedMessages.message;
	debugLog(`Completed encrypting messages.`);

	// Test decryption.
	debugLog(`Decrypting ${encryptedMessages.length} messages.`);
	let decryptedMessages = decryptRSAMessages(encryptedMessages, keyPair.private);
	if (!decryptedMessages.success) return debugLog(decryptedMessages.message);
	decryptedMessages = decryptedMessages.message;
	debugLog(`Completed decrypting messages.`);

	// Test invalid encryption
	// Due to design limitations, RSA has an upper limit of the length of encrypted test.
	// These should fail with a good error message.
	debugLog(`Testing encryption of too long messages.`);
	const TOO_LONG_MESSAGES = generateNewMessages(3, 5000, 4000);
	let tooLongEncryptedMessages = encryptRSAMessages(TOO_LONG_MESSAGES, keyPair.public, true);
	if (tooLongEncryptedMessages.success) return debugLog(`Messages that should not be able to be encrypted were encrypted. Something went wrong!`);
	debugLog(`Invalid encryption messages tested!`);


	// Test invalid public key encryption
	// A invalid key of some kind is provided. which should fail.
	debugLog(`Testing encryption using invalid public key.`);
	let invalidPublicKeyEncryption = encryptRSAMessages(MESSAGES_TO_ENCRYPT, "I am not a public key, but here I am.", true);
	if (invalidPublicKeyEncryption.success) return debugLog(`Messages that should not be able to be encrypted were encrypted. Something went wrong!`);
	debugLog(`Invalid public key messages tested!`);

	// Test decrypting invalid messages
	// This could just be messages that are not encrypted. 
	debugLog("Testing decryption of invalid messages.");
	let notValidEncryptedMessages = decryptRSAMessages(MESSAGES_TO_ENCRYPT, keyPair.private, true);
	if (notValidEncryptedMessages.success) return debugLog(`Messages that should not be able to be decrypted were decrypted. Something went wrong!`);
	debugLog("Testing decryption of invalid messages completed.");

	// This round of testing was a success! :D
	debugLog(`Testing RSA key bit length of ${bitLength} was successful.`);
	debugLog(`---`);
}

module.exports = { runRSATest }

function generateNewMessages(amount = 5, maxLengthPerMessage = 3000, minLengthPerMessage = 30) {
	let messages = [];

	for (let i = 0; amount > i; i++) {
		// We want a long message. Get a message of length random that can be a large number.
		const NEW_MESSAGE = generateStringOfLength(generateRandomNumber(maxLengthPerMessage, minLengthPerMessage));

		messages.push(NEW_MESSAGE);
	}

	debugLog(`Generated ${amount} messages.`)

	return messages;
}

function generateStringOfLength(length) {
	const CHAR_SET = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ';

	let result = '';
	const CHAR_SET_LENGTH = CHAR_SET.length;

	for (let i = 0; i < length; i++) {
		const RANDOM_INDEX = Math.floor(Math.random() * CHAR_SET_LENGTH);
		result += CHAR_SET[RANDOM_INDEX];
	}

	return result;
}

function generateRSAKeyPair(bitLength) {
	const RSA_KEY_PAIR = crypto.generateRSAKeypair(bitLength);

	if (!RSA_KEY_PAIR) return buildResponse(`Invalid RSA key generated. Check C++.`, false);
	if (typeof RSA_KEY_PAIR !== "object") return buildResponse(`Invalid type of RSA key generated. Not an object.`, false);
	if (!RSA_KEY_PAIR.public) return buildResponse(`Invalid RSA content. No Public key provided.`, false);
	if (!RSA_KEY_PAIR.private) return buildResponse(`Invalid RSA content. No private key provided.`, false);

	debugLog(`Generated a RSA key pair of ${bitLength} bits.`);

	return buildResponse(RSA_KEY_PAIR);
}

function generateSignaturesFromMessages(messages = [], privateKeyPEM) {
	let messageSignatures = [];
	let successfullyGeneratedSignatures = true; // Assume we are successful.

	for (let messageEntry of messages) {
		const MESSAGE_SIGNATURE = generateRSAMessageSignature(messageEntry, privateKeyPEM);

		if (!MESSAGE_SIGNATURE.success) {
			// One of the RSA message signatures was unsuccessful.
			successfullyGeneratedSignatures = false;
			debugLog(`Failed to sign the following message:\n${messageEntry}\nWe tried to sign it using the private key:\n${privateKeyPEM}\n`)
			break;
		}

		messageSignatures.push(MESSAGE_SIGNATURE.message)
	}

	if (!successfullyGeneratedSignatures) return buildResponse(`Failed to generate a signature. See error above.`, false);

	debugLog(`Successfully generated ${messageSignatures.length} / ${messages.length} signatures.`);

	return buildResponse(messageSignatures);
}

function generateRSAMessageSignature(messageToSign, privateKey) {
	const MESSAGE_SIGNATURE = crypto.signRSAMessage(messageToSign, privateKey);

	if (!MESSAGE_SIGNATURE) return buildResponse(`Invalid RSA message signature generated. Check C++.`, false);
	if (typeof MESSAGE_SIGNATURE !== "string") return buildResponse(`Invalid type of RSA message signature generated. Not a string.`, false);

	return buildResponse(MESSAGE_SIGNATURE);
}

function testSignaturesFromMessages(messages = [], signatures = [], publicKey, failQuieter = false) {
	if (messages.length !== signatures.length) return buildResponse(`Failed testing signatures. Message length does not equal signatures.`, false);

	let successfullyVerifiedAllSignatures = true; // Assume we are successful.

	// We can do this because we verified the length of signatures and the messages are equal as they should be.
	for (let i = 0; messages.length > i; i++) {
		const MESSAGE_SIGNATURE_IS_VALID = testRSAKeySignature(messages[i], signatures[i], publicKey);
		if (!MESSAGE_SIGNATURE_IS_VALID.success) {
			// Message signature failed to validate.
			// Even if the validation fails, this does not strictly mean that the functionality of message verification is broken or does not work.
			// This means we have to handle this differently compared to generating the message signatures.

			// Firstly, we still set this to false, because we did fail to verify the message with the provided signature.
			successfullyVerifiedAllSignatures = false;

			// We'll log the message verification here too.
			debugLog(`Failed to verify a message!${failQuieter ? "" : `\nMessage: '${messages[i]}'\n\Signature:\n'${signatures[i]}'.`}`);

			// And then we continue executing through the other signatures.

			// We do test what should be known valid signatures/messages combinations, and known invalid signature/messages combinations.
			// To keep things simple, just return a boolean and a success / error message.
		}
	}

	if (!successfullyVerifiedAllSignatures) return buildResponse(`Some message(s) failed to verify!`, false);
	return buildResponse(true);
}

function testRSAKeySignature(message, messageSignature, publicKey) {
	const IS_MESSAGE_AND_SIGNATURE_VALID = crypto.validateRSASignature(message, messageSignature, publicKey);

	if (typeof IS_MESSAGE_AND_SIGNATURE_VALID !== "boolean") return buildResponse(`Invalid RSA message signature validation value. Not a boolean.`, false);

	return buildResponse(null, IS_MESSAGE_AND_SIGNATURE_VALID);
}

function encryptRSAMessages(messages = [], publicKey, failQuieter = false) {
	let encryptedMessages = [];
	let successfullyEncryptedAllMessages = true; // Assume success.

	for (let i = 0; messages.length > i; i++) {
		const ENCRYPTED_MESSAGE = encryptMessage(messages[i], publicKey);

		if (!ENCRYPTED_MESSAGE.success) {
			successfullyEncryptedAllMessages = false;
			if (failQuieter) {
				debugLog(`Failed to encrypt a message!`);
			} else {
				debugLog(`Failed to encrypt a message!\nMessage: '${messages[i]}'\n\nPublic Key: '${publicKey}'`);
				debugLog(`Failed with error: ${ENCRYPTED_MESSAGE.message}`);
			}
		}

		encryptedMessages.push(ENCRYPTED_MESSAGE.message);
	}

	if (!successfullyEncryptedAllMessages) return buildResponse(`Failed to encrypt some messages!`, false);
	return buildResponse(encryptedMessages);
}

function encryptMessage(message, publicKey) {
	const ENCRYPTED_MESSAGE = crypto.encryptRSA(message, publicKey);

	if (!ENCRYPTED_MESSAGE) return buildResponse(`Invalid RSA encryption generated. Check C++`, false);
	if (typeof ENCRYPTED_MESSAGE !== "string") return buildResponse(`Invalid RSA encryption type generated. Type '${typeof ENCRYPTED_MESSAGE}' is not expected.`, false);

	return buildResponse(ENCRYPTED_MESSAGE);
}

function decryptRSAMessages(messages = [], privateKey, failQuieter = false) {
	let decryptedMessages = [];
	let successfullyDecryptedAllMessages = true; // Assume success.

	for (let i = 0; messages.length > i; i++) {
		const DECRYPTED_MESSAGE = decryptMessage(messages[i], privateKey);

		if (!DECRYPTED_MESSAGE.success) {
			successfullyDecryptedAllMessages = false;
			debugLog(`Failed to decrypt a message!${failQuieter ? "" : `\nMessage: '${messages[i]}'\n\Signature:\n'${signatures[i]}'.`}`)
		}

		decryptedMessages.push(DECRYPTED_MESSAGE);
	}

	if (!successfullyDecryptedAllMessages) return buildResponse(`Failed to decrypt some messages!`, false);
	return buildResponse(decryptedMessages);
}

function decryptMessage(message, privateKey) {
	const DECRYPTED_MESSAGE = crypto.decryptRSA(message, privateKey);

	if (!DECRYPTED_MESSAGE) return buildResponse(`Invalid RSA decryption generated. Check C++`, false);
	if (typeof DECRYPTED_MESSAGE !== "string") return buildResponse(`Invalid RSA decryption type generated. Type '${typeof DECRYPTED_MESSAGE}' is not expected.`, false);

	return buildResponse(DECRYPTED_MESSAGE);
}

function buildResponse(message = "Failure", success = true) {
	return { success: success, message: message };
}
