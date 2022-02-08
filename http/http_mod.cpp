// Written by Gil Dabah.
// http://ragestorm.net 2003
// Simple HTTP/1.1 module which sends GET/POST requests and saves the result in "output.txt".

// Usage example:


//int SendHTTPRequest(const char* address, const char* URI,
//		RequestMethodT requestMethod, int paramsNo, char* paramsNames[], char* values[],
//		const char* hostName, int defaultPort)


// Simply GETting the index file:
// SendHTTPRequest("www.ragestorm.net", "/", REQUEST_GET, 0, NULL, NULL, "www.ragestorm.net");

// POSTing ("posting.php" doesn't really exist!):
// const int PARAMSNO = 2;
// char paramNames[] = {"arg1", "arg2"};
// char paramValues[] = {"Hello World", "2003"};
// SendHTTPRequest("www.ragestorm.net", "/posting.php", REQUEST_POST, PARAMSNO, paramNames, paramValues, "www.ragestorm.net");


// Feel free to do anything you wish with it.
// ENJOY!

#include "http_mod.h"
#include <windows.h>

#define MAX_STR (2048)


// Comment it out if you wanna see messages which inform what's going on...
// #define HTTP_MOD_TRACE_OUTPUT

// Escape a given string, according to RFC 2396(URI Generic Syntax)
static string EscapeString(const char* s)
{
	// Those characters can stay in the "escaped" form.
	char UnescapedCharacters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.!~*'()";
	string ret = "";
	char hexBuf[3] = "";

	// Scan all characters.
	for (DWORD i = 0; i < strlen(s); i++) {
		// Check if the specific character should be escaped.
		for (int j = 0; j < sizeof(UnescapedCharacters); j++)
			if (s[i] == UnescapedCharacters[j]) break;
		// It wasn't found in the table, so escape it:
		if (j == sizeof(UnescapedCharacters)) {
			// Add the Escaping Delimitor.
			ret += "%";
			// Convert the ASCII value into HEX.
			hexBuf[2] = '\0';
			itoa(s[i] & 0xff, hexBuf, 16);
			// Add it to the buffer.
			ret += hexBuf;
		} else ret += s[i]; // If it shouldn't be escaped, just add it to the buffer as is
	}
	return ret;
}

// Parse response
// HTTP-Version SP Status-Code SP Reason-Phrase CRLF
static int ParseHTTPResponse(string& recvData)
{
	// Make sure the response is full response and in HTTP/1.1 format.
	if (recvData.compare(0, strlen(HTTP_FULL_HEADER), HTTP_FULL_HEADER) != 0) { // Not HTTP header...
#if defined (HTTP_MOD_TRACE_OUTPUT)
		MessageBox(0, "Response without HTTP header!", "HTTP_MOD", 0);
#endif
		return FALSE;
	}

	// Extract status code:
	string responseCodeStr = recvData.substr(strlen(HTTP_FULL_HEADER), HTTP_STATUS_CODE_LENGTH);
	char* invalidCh = NULL;
	// The status code is in decimal.
	errno = 0;
	long responseCode = strtol(responseCodeStr.c_str(), &invalidCh, 10);
	// Overflow in number?
	if (((responseCode == LONG_MAX) || (responseCode == LONG_MIN)) && (errno == ERANGE)) {
#if defined (HTTP_MOD_TRACE_OUTPUT)
		MessageBox(0, "Overflow reading status code!", "HTTP_MOD", 0);
#endif
		return FALSE;
	}
	if (*invalidCh != '\0') { // Error reading number, should be ended with Null Terminate.
#if defined (HTTP_MOD_TRACE_OUTPUT)
		MessageBox(0, "Cannot read status code!", "HTTP_MOD", 0);
#endif
		return FALSE;
	}

// Ignore the Response-Phrase.
// As long as we found the CRLF at the end of the response, we are satisfied with extracting the status code.

#if defined (HTTP_MOD_TRACE_OUTPUT)
	if (responseCode != HTTP_RESPONSE_SUCCESSFUL_OK) {
		MessageBox(0, "Response code not OK!", "HTTP_MOD", 0);
	}
#endif

	return responseCode;
}

// Incoming parameters:
// address: address of server to connect (can be both IP or a domain name).
// URI: resource on server
// paramsNo: number of parameters to send within URL
// requestMethod: which request you would like to send (GET/POST)
// paramsNames: the parameters names to send
// values: the values of the parameters to send
// hostName: the name in ASCII of the server (can be empty on most servers)
// defaultPort: if you want to use some other port, use this

// Return value: FALSE - An error occurred
//	TRUE - Everything was fine but response code
// anything else is the successful response code OK
int SendHTTPRequest(const char* address, const char* URI,
		RequestMethodT requestMethod, int paramsNo, char* paramsNames[], char* values[],
		const char* hostName, int defaultPort)
{
	// Build the request packet according to protocol 1.1.

	// rawData will contain the packet with http1.1 protocol to send.
 string rawData = "";
	// Fill in the wanted request.
	switch (requestMethod)
	{
		case REQUEST_GET:
			rawData += HTTP_GET_REQUEST;
		break;
		case REQUEST_POST:
			rawData += HTTP_POST_REQUEST;
		break;
	}

	rawData += HTTP_SP;
	rawData += URI;

	// If we're in GET request, it's time to supplement the parameters.
	if (paramsNo > 0 && requestMethod == REQUEST_GET) {
		// Add the parameters as query and escape them.
		rawData += '?'; // Query character.
		for (int i = 0; i < paramsNo; i++) {
			rawData += paramsNames[i]; // Parameter name.
			rawData += "="; // Assignment character.
			rawData += EscapeString(values[i]); // Add the escaped value.
			if (i < paramsNo - 1) rawData += '&'; // Next parameter.
		}
	}
	rawData += HTTP_SP;
	rawData += HTTP_FULL_HEADER;
	rawData += HTTP_CRLF;
	// Accept anything.
	rawData += HTTP_ACCEPT;
	// We want english only.
	rawData += HTTP_ACCEPT_LANG;
	// Close connection after response.
	rawData += HTTP_CONNECTION_CLOSED;
	// Host name.
	rawData += HTTP_HOST;
	rawData += HTTP_SP;
	rawData += hostName;
	rawData += HTTP_CRLF;

	// Send parameters if it's in POST method.
	if (paramsNo > 0 && requestMethod == REQUEST_POST) {
		// Add the content type for posting.
		rawData += HTTP_CONTENT_TYPE;
		rawData += HTTP_CONTENT_TYPE_DEF;
		rawData += HTTP_CRLF;

		// Create the query string first.
		string query = "";
		for (int i = 0; i < paramsNo; i++) {
			query += paramsNames[i]; // Parameter name.
			query += "="; // Assignment character.
			query += EscapeString(values[i]); // Add the escaped value.
			if (i < paramsNo - 1) query += '&'; // Next parameter.
		}

		// So we have its length.
		// Post request has to contain content length, so calculate it.
		rawData += HTTP_CONTENT_LENGTH;
		char lengthBuf[16] = {0};
		// Deciaml base.
		itoa((int)query.length(), lengthBuf, 10);
		rawData += lengthBuf;
		// Notice TWO CRLF's, because the query itself in POST is the Entity-Body!
		rawData += HTTP_CRLF;
		rawData += HTTP_CRLF;

		// Entity-Body is the query string:
		rawData += query;
	}

	// End of request.
	rawData += HTTP_CRLF;

#if defined (HTTP_MOD_TRACE_OUTPUT)
	MessageBox(0, rawData.c_str(), "HTTP_MOD", 0);
#endif

	// Fill in the SOCKADDR_IN structure
	SOCKADDR_IN server_info = {0};

	// First try interpreting the address as IP
	server_info.sin_addr.s_addr = inet_addr(address);
	if (server_info.sin_addr.s_addr == INADDR_NONE) { // Uh h

		// Now it's might be an address, DNS is coming in...
		hostent* server_host = gethostbyname(address);
		if (server_host != NULL) {
			server_info.sin_addr = *(in_addr *)server_host->h_addr;
		} else {
#if defined (HTTP_MOD_TRACE_OUTPUT)
			MessageBox(0, "Given host is not found!", "HTTP_MOD", 0);
#endif
			return FALSE;
		}
	}

	server_info.sin_family = AF_INET;
 server_info.sin_port = htons(defaultPort); // HTTP port.
 SOCKET http_sock = socket(AF_INET, SOCK_STREAM, 0); // TCP based connection.
	if (http_sock ==  INVALID_SOCKET) {
#if defined (HTTP_MOD_TRACE_OUTPUT)
		MessageBox(0, "Cannot create socket!", "HTTP_MOD", 0);
#endif
		return FALSE;
	}

	// Try to connect...
	if (connect(http_sock, (SOCKADDR*)&server_info, sizeof(server_info)) == SOCKET_ERROR) {
#if defined (HTTP_MOD_TRACE_OUTPUT)
		MessageBox(0, "Cannot connect to server!", "HTTP_MOD", 0);
#endif
		closesocket(http_sock);
		return FALSE;
	}


	// Because it's a byte stream, we have to make sure all data has been sent well, so you would usually use a WHILE statement
	// to complete that, but it's unnecessary, because the socket is in blocking mode!
	if (send(http_sock, rawData.c_str(), (int)rawData.length(), 0) == SOCKET_ERROR) {
#if defined (HTTP_MOD_TRACE_OUTPUT)
		MessageBox(0, "Cannot send packet to server!", "HTTP_MOD", 0);
#endif
		closesocket(http_sock);
		return FALSE;
	}

	// Receive the response (this is the meat of the function)

	// This will hold incoming raw data from server
	string recvData = "";

	// Hold immediate incoming data from server(by recv).
	char buf[MAX_STR+1] = {0};
	int ret = 0;
	// Indicates we are reading and processing headers in response
	bool isInHeaders = true;
	// Indicates we are reading an HTTP/1.1 full response
	bool isFullResponse = false;
	// Hold positions for basic_string::find...
	basic_string <char>::size_type pos = 0;
	// The position of the end of the headers chunk.
	basic_string <char>::size_type endOfHeadersPos = 0;
	// "String wasn't found" value.
	static const basic_string <char>::size_type not_found_pos = -1;
	// This tells us in which way to extract the incoming data according to the server
	dataMethodT dataMethod = C_NONE;
	// If we extract the data according to a given length, hold that length amount.
	DWORD contentLength = -1;

	while (1) {
		ZeroMemory(buf, MAX_STR+1);
		// Read incoming raw data from server, blocking IO.
		ret = recv(http_sock, buf, MAX_STR, 0);
#if defined (HTTP_MOD_TRACE_OUTPUT)
		MessageBox(0, buf, "received this: ", 0);
#endif
		if (ret == 0) {// Socket was closed gracefully.
			break;
		} else if (ret == SOCKET_ERROR) {
			// We ignore arrived input.
#if defined (HTTP_MOD_TRACE_OUTPUT)
			MessageBox(0, "Cannot receive packet from server!", "HTTP_MOD", 0);
#endif
			// Drop connection.
			closesocket(http_sock);
			return FALSE;
		}
		// Convert any null terminating characters to spaces.
		// I stumbled some stupid server that sends \0 ! ! !
		for (int i = 0; i < ret - 1; i++)
			if (buf[i] == '\0') buf[i] = HTTP_SP;
		// Concatenate the new data received.
		recvData += buf;

		// Make sure it's an HTTP/1.1 response first, by minimum length, containing a CRLF at the end, compare to HTTP header string.
		// This should be enough to say we've got a full response, hopefully.
 	if ((!isFullResponse) && (recvData.length() >= (strlen(HTTP_FULL_HEADER) + HTTP_STATUS_CODE_LENGTH)) && (recvData.find(HTTP_CRLF) != not_found_pos)) {
#if defined (HTTP_MOD_TRACE_OUTPUT)
			MessageBox(0, recvData.c_str(), "HTTP_MOD headers", 0);
#endif
			// If we got CRLF it means we should have a full response, so we can start parsing it.
			int resCode = ParseHTTPResponse(recvData);
			if (resCode != HTTP_RESPONSE_SUCCESSFUL_OK) {
				// Give a chance to "100 Continue" response/
				if (resCode == HTTP_RESPONSE_INFORMATIONAL_CONTINUE) {
					// If it is "continue" then ignore it, don't forget it comes with two pairs of CRLF's!
					// Leave the rest of the data, if any.
					recvData = recvData.substr(pos + strlen(HTTP_CRLFCRLF));
					continue; // and Continue *grins*
				} else {
					// We don't support that status code...
					closesocket(http_sock);
					return FALSE;
				}
			} else isFullResponse = true; // We got a full response.
		}
		
		// Make sure we are in headers data, otherwise ignore it.
		if (isFullResponse && isInHeaders) {

			// We search for two pairs of CRLF's, indicates the start of Entity-Body or end of headers.
			endOfHeadersPos = recvData.find(HTTP_CRLFCRLF);
			if (endOfHeadersPos != not_found_pos) { // We got all headers.

				// Mark that all headers were received, so we can start parsing'em only once.
				isInHeaders = false;

				// Check to see if it has a length.
				pos = recvData.find(HTTP_CONTENT_LENGTH);
				if (pos != not_found_pos && pos < endOfHeadersPos) { // Make sure it's in headers, because it could be in the data content.

					// Extract the length(decimal byte units):
					pos += strlen(HTTP_CONTENT_LENGTH);
					char* invalidCh = NULL;
					// The length is in decimal.
					errno = 0;
					contentLength = strtol(&recvData.c_str()[pos], &invalidCh, 10);
					// Overflow in number?
					if (((contentLength == LONG_MAX) || (contentLength == LONG_MIN)) && (errno == ERANGE)) {
#if defined (HTTP_MOD_TRACE_OUTPUT)
						MessageBox(0, "Overflow reading length!", "HTTP_MOD", 0);
#endif
						closesocket(http_sock);
						return FALSE;
					}
					// The length should be ended by a CRLF.
					if (*invalidCh != HTTP_CR) {
#if defined (HTTP_MOD_TRACE_OUTPUT)
						MessageBox(0, "Invalid length number!", "HTTP_MOD", 0);
#endif
						closesocket(http_sock);
						return FALSE;
					}

#if defined (HTTP_MOD_TRACE_OUTPUT)
					char buff[10] = {0};
					sprintf(buff, "Length is: %d", contentLength);
					MessageBox(0, buff, "HTTP_MOD", 0);
#endif
					dataMethod = C_LENGTH; // Mark that we found a content-length header.
				}

				// Give a chance to chunked mode:
				pos = recvData.find(HTTP_TRANSFER_ENCODING_CHUNKED);
				if (pos != not_found_pos && pos < endOfHeadersPos) { // Make sure it's in headers.

#if defined (HTTP_MOD_TRACE_OUTPUT)
					MessageBox(0, "It's in chunked mode!", "HTTP_MOD", 0);
#endif     
					// According to the RFC:
					// "Messages must not include both a Content-Length header field and the "chunked" transfer coding.
					// If both are received, the Content-Length must be ignored."
					// So if we got here let's make it C_CHUNKED ONLY.
					dataMethod = C_CHUNKED;
				}

				break; // We got all headers, let's start extracting!
			}
		}
	}

	// Holds the cleaned data, after parsing the chunks or concatenating packets.
	string extractedData = "";

	if (dataMethod == C_NONE) { // Extract the incoming data till the socket is closed by the SERVER.
		// Copy the bytes we got already (after headers, if any) and then continue receiving.
		extractedData = recvData.substr(endOfHeadersPos + strlen(HTTP_CRLFCRLF));
		while (1) {
			ZeroMemory(buf, MAX_STR+1);
			// Read incoming raw data from server, blocking IO.
			ret = recv(http_sock, buf, MAX_STR, 0);
			if (ret == 0) {// Socket was closed gracefully.
				break;
			} else if (ret == SOCKET_ERROR) {
				// We ignore arrived input.
#if defined (HTTP_MOD_TRACE_OUTPUT)
				char b[10] = {0};
				itoa(WSAGetLastError(), b, 10);
				MessageBox(0, "Cannot receive packet from server!", b, 0);
#endif
				break; // We still got something...
			}

			// Convert any null terminating characters to spaces
			for (int i = 0; i < ret - 1; i++) 
				if (buf[i] == '\0') buf[i] = HTTP_SP;
			// Concatenate received data.
			extractedData += buf;
		}
#if defined (HTTP_MOD_TRACE_OUTPUT)
		MessageBox(0, extractedData.c_str(), "HTTP_MODE NONE", 0);
#endif
	} else 	if (dataMethod == C_LENGTH) { // Extract incoming data by length.

		// Copy the bytes we got already (after headers, if any) and then continue receiving.
		extractedData = recvData.substr(endOfHeadersPos + strlen(HTTP_CRLFCRLF));
		// Calculate the length of what we got now.
		DWORD receivedDataLength = (DWORD)extractedData.length();
		// Receive the rest of the data, if any:
		while (receivedDataLength < contentLength) {

			ZeroMemory(buf, MAX_STR+1);
			// Read incoming raw data from server, blocking IO
			ret = recv(http_sock, buf, MAX_STR, 0);
			if (ret == 0) {// Socket was closed gracefully
				break;
			} else if (ret == SOCKET_ERROR) {
				// We ignore arrived input
#if defined (HTTP_MOD_TRACE_OUTPUT)
				char b[10] = {0};
				itoa(WSAGetLastError(), b, 10);
				MessageBox(0, "Cannot receive packet from server!", b, 0);
#endif
				break; // We still got something...
			}

			// Calculate the length, seperated from the string.
			receivedDataLength += ret;
			if (receivedDataLength > contentLength) {
				// Data exceeds given length, drop connection!!!
				// RFC says that in such a case we have to inform the user, oh really? :)
#if defined (HTTP_MOD_TRACE_OUTPUT)
				MessageBox(0, "Data exceeds the given length!", "HTTP_MOD", 0);
#endif
				closesocket(http_sock);
				return FALSE;
			}

			// Convert any null terminating characters to spaces
			for (int i = 0; i < ret - 1; i++) 
				if (buf[i] == '\0') buf[i] = HTTP_SP;
			extractedData += buf;
		}

	} else if (dataMethod == C_CHUNKED) { // Extract incoming data by chunked mode.

		// Chunk : <SIZE IN HEX> CRLF <DATA> CRLF
		DWORD currentChunkSize = -1, receivedDataLength = 0;
		// nextPos will help us parsing the data, and points to the next pos we start examining the buffer from.
		basic_string <char>::size_type nextPos = endOfHeadersPos + strlen(HTTP_CRLFCRLF);
		// When we are not reading the chunk data, we are searching for the next chunk-size.
		bool isReadingChunk = false;

		// Read chunk by chunk, 'till we get one with the size of 0!
		// Reading chunks is pretty confusing so follow the code tightly:
		while (1) {

			// Are we searching for a chunk-size?
			if (!isReadingChunk) {

				// Extract <chunk-size>
				// If we find a CRLF pair after data, it means we have the size ready to be read...
				pos = recvData.find(HTTP_CRLF, nextPos);
				if (pos != not_found_pos) {

#if defined (HTTP_MOD_TRACE_OUTPUT)
					MessageBox(0, &recvData.c_str()[nextPos], "AT CHUNK-SIZE", 0);
#endif
					char* invalidCh = NULL;
					// The size is stored in HEX
					errno = 0;
					currentChunkSize = strtol(&recvData.c_str()[nextPos], &invalidCh, 16);
					// Overflow in number?
					if (((currentChunkSize == LONG_MAX) || (currentChunkSize == LONG_MIN)) && (errno == ERANGE)) {
#if defined (HTTP_MOD_TRACE_OUTPUT)
						MessageBox(0, "Overflow reading chunk size!", "HTTP_MOD", 0);
#endif
						return FALSE;
						closesocket(http_sock);
					}
					// According to some references the chunk size can be prefixed to some ignore-able data which is seperated by ';' character.
					if ((*invalidCh != HTTP_CR) && (*invalidCh != ';') && (*invalidCh != HTTP_SP)) {

#if defined (HTTP_MOD_TRACE_OUTPUT)
						MessageBox(0, "Error upon reading Chunk-Size!", "HTTP_MOD", 0);
#endif
						closesocket(http_sock);
						return FALSE;
					}

					// Last chunk???
					if (currentChunkSize == 0) {
#if defined (HTTP_MOD_TRACE_OUTPUT)
						MessageBox(0, "Last chunk was found!", "HTTP_MOD", 0);
#endif
						break;
					}

					// Raw data begins at nextPos.
					nextPos = pos + strlen(HTTP_CRLF);
					// So read the data.
					isReadingChunk = true;
					// Calculate left length.
					receivedDataLength = (DWORD)recvData.substr(nextPos).length();
				}
			}

			// We got the whole chunk?
			if (isReadingChunk && receivedDataLength >= currentChunkSize) {

				// We are looking for the next chunk-size...
				isReadingChunk = false;

				// Check to see if the chunk size is valid,
				// a CRLF pair should be found immediately after the <chunk-size> bytes!
				if (recvData.compare(nextPos + currentChunkSize, strlen(HTTP_CRLF), HTTP_CRLF) != 0) {
#if defined (HTTP_MOD_TRACE_OUTPUT)
					MessageBox(0, "Chunk-Data wasn't closed by a CRLF pair!", "HTTP_MOD", 0);
#endif
					closesocket(http_sock);
					return FALSE;
				}

				// Here, we know the chunk is alright, so concatenate it to the cleaned buffer.
				extractedData += recvData.substr(nextPos, currentChunkSize);
				// Skip <chunk-data> and CRLF pair.
				nextPos += currentChunkSize + strlen(HTTP_CRLF);
			} else { // Need to read more data.
				// We read data from socket as the last thing,
				// because it could be that we got two chunks in one packet,
				// so we have to parse them both before continuing reading more data.

				ZeroMemory(buf, MAX_STR+1);
				// Read incoming raw data from server, blocking IO.
				ret = recv(http_sock, buf, MAX_STR, 0);
				if (ret == 0) {// Socket was closed gracefully.
					break;
				} else if (ret == SOCKET_ERROR) {
					// We ignore arrived input.
#if defined (HTTP_MOD_TRACE_OUTPUT)
					MessageBox(0, "Cannot receive packet from server!", "HTTP_MOD", 0);
#endif
					break; // We still got something...
				}

				// Convert any null terminating characters to spaces
				for (int i = 0; i < ret - 1; i++) 
					if (buf[i] == '\0') buf[i] = HTTP_SP;
				recvData += buf;
				receivedDataLength += ret;
			}
		}
	}
	// We got in here, alrighta then!

	// Write the result to some file.
	HANDLE hFile = CreateFile("output.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		MessageBox(0, "Cannot create output file!", "HTTP_MOD", 0);
	}
	DWORD bytesWritten = 0;
	WriteFile(hFile, extractedData.c_str(), (DWORD)extractedData.length(), &bytesWritten, NULL);
	CloseHandle(hFile);

	closesocket(http_sock);
#if defined (HTTP_MOD_TRACE_OUTPUT)
	MessageBox(0, "Closing connection!", "HTTP_MOD", 0);
#endif
	return HTTP_RESPONSE_SUCCESSFUL_OK;
}