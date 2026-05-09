import axios, { AxiosInstance } from "axios";

export class UrlShortenerClient {
  private client: AxiosInstance;

  constructor(host: string, port: number) {
    this.client = axios.create({
      baseURL: `http://${host}:${port}`,
    });
  }

  private validateUrl(url: string): void {
    if (!url.startsWith('http://') && !url.startsWith('https://')) {
      throw new Error('URL must start with http:// or https://');
    }
  }

  async shorten(originalUrl: string): Promise<string> {
    try {
    this.validateUrl(originalUrl);
    console.log("[CLIENT] shorten request:", originalUrl);
    const response = await this.client.post("/shorten", {
      url: originalUrl,
    });

    if (response.status !== 200 && response.status !== 201) {
      throw new Error("Unexpected response");
    }

    return response.data.code;
      
    } catch (err) {
      throw new Error("Failed to shorten URL");
    }
  }


  async resolve(shortCode: string): Promise<string> {
    try {
    console.log("[CLIENT] resolve request:", shortCode);
    const response = await this.client.get(
      `/resolve/${shortCode}`
    );

    if (response.status !== 200 && response.status !== 201) {
      throw new Error("Unexpected response");
    }

    return response.data.url;
    } catch (err) {
      throw new Error("Failed to resolve URL");
    }
  }

  async remove(shortCode: string): Promise<void> {
    try {
      console.log("[CLIENT] remove request:", shortCode);
      const response = await this.client.delete(
        `/remove/${shortCode}`
      );

      if (response.status !== 200 && response.status !== 201) {
        throw new Error("Unexpected response");
      }
    } catch (err) {
      throw new Error("Failed to remove URL");
    }
  }
}