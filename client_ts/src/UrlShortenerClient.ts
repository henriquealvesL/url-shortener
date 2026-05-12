import * as net from "net";

export class UrlShortenerClient {
  private host: string;
  private port: number;

  constructor(host: string, port: number) {
    this.host = host;
    this.port = port;
  }

  private validateUrl(url: string): void {
    if (!url.startsWith('http://') && !url.startsWith('https://')) {
      throw new Error('URL must start with http:// or https://');
    }
  }

  private async sendCommand(command: string): Promise<string> {
    return new Promise((resolve, reject) => {
      const socket = new net.Socket();
      let response = '';

      socket.connect(this.port, this.host, () => {
        socket.write(command + '\n');
      });

      socket.on('data', (data) => {
        response += data.toString();
        if (response.includes('\n')) {
          socket.destroy();
        }
      });

      socket.on('close', () => {
        const trimmedResponse = response.trim();
        if (trimmedResponse.startsWith('ERR ')) {
          const parts = trimmedResponse.split(' ', 3);
          const statusCode = parseInt(parts[1]);
          const message = parts.slice(2).join(' ');
          reject(new Error(`Proxy error ${statusCode}: ${message}`));
        } else if (trimmedResponse.startsWith('OK ')) {
          const parts = trimmedResponse.split(' ');
          resolve(parts.slice(1).join(' '));
        } else if (trimmedResponse === 'OK') {
          resolve('');
        } else {
          reject(new Error('Unexpected response from proxy'));
        }
      });

      socket.on('error', (err) => {
        reject(new Error(`Connection error: ${err.message}`));
      });
    });
  }

  async shorten(originalUrl: string): Promise<string> {
    try {
      this.validateUrl(originalUrl);
      console.log("[CLIENT] Connected to proxy localhost:9000");
      console.log("[CLIENT] shorten request:", originalUrl);
      const response = await this.sendCommand(`ENCURTA ${originalUrl}`);
      const parts = response.split(' ', 2);
      return parts[0]; // Return just the short code
    } catch (err) {
      // Preserve the original error message for better debugging
      if (err instanceof Error) {
        throw new Error(`Failed to shorten URL: ${err.message}`);
      }
      throw new Error("Failed to shorten URL");
    }
  }

  async resolve(shortCode: string): Promise<string> {
    try {
      console.log("[CLIENT] resolve request:", shortCode);
      const response = await this.sendCommand(`RESOLVE ${shortCode}`);
      return response; // Return the original URL
    } catch (err) {
      // Preserve the original error message for better debugging
      if (err instanceof Error) {
        throw new Error(`Failed to resolve URL: ${err.message}`);
      }
      throw new Error("Failed to resolve URL");
    }
  }

  async remove(shortCode: string): Promise<void> {
    try {
      console.log("[CLIENT] remove request:", shortCode);
      await this.sendCommand(`REMOVE ${shortCode}`);
    } catch (err) {
      // Preserve the original error message for better debugging
      if (err instanceof Error) {
        throw new Error(`Failed to remove URL: ${err.message}`);
      }
      throw new Error("Failed to remove URL");
    }
  }
}