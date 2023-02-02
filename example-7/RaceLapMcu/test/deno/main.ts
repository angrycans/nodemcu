
import { Application, Router, send } from "https://deno.land/x/oak/mod.ts";
import { oakCors } from "https://deno.land/x/cors/mod.ts";


const serverURL = "https://parseapi.back4app.com";



const app = new Application();
app.use(oakCors()); 
app.use(async (ctx) => {
   const result = ctx.request.body({ type: "text" });
   const text = await result.value;

  const res=await fetch(serverURL+ctx.request.url.pathname, {
    "headers": {
      "accept": "*/*",
      "accept-language": "zh-CN,zh;q=0.9,en;q=0.8",
      "content-type": "text/plain",
      "sec-ch-ua": "\"Not_A Brand\";v=\"99\", \"Google Chrome\";v=\"109\", \"Chromium\";v=\"109\"",
      "sec-ch-ua-mobile": "?0",
      "sec-ch-ua-platform": "\"macOS\"",
      "sec-fetch-dest": "empty",
      "sec-fetch-mode": "cors",
      "sec-fetch-site": "cross-site"
    },
    "referrerPolicy": "strict-origin-when-cross-origin",
    "body":text,
    "method": "POST",
    "mode": "cors",
    "credentials": "omit"
  });
  
    ctx.response.body=await res.json();

  });


console.info("CORS-enabled web server listening on port 80");
await app.listen({ port: 80 });
