import { UrlShortenerClient } from "./UrlShortenerClient";

async function main() {
  const client = new UrlShortenerClient(
    "localhost",
    8080
  );

  const code = await client.shorten(
    "https://google.com"
  );

  console.log("Short code:", code);

  const url = await client.resolve(code);

  console.log("Resolved URL:", url);

  await client.remove(code);

  console.log("URL removed");
}

main();