
import { Application, Router, send } from "https://deno.land/x/oak/mod.ts";
import { oakCors } from "https://deno.land/x/cors/mod.ts";


const serverURL = "https://parseapi.back4app.com";

(async ()=>{

    console.log("test start");
    const text=new URLSearchParams({
        'grant_type': 'refresh_token',
        'client_id': '054fu81io7bpqjm',
        'client_secret': 'fkjvl5t718xyj16',
        'refresh_token': '7n_Dt3I2izUAAAAAAAAAAbno9CplSr8CcdFYeMkA-1-Uuf-FX2fWks0ep8Xx7npL'
    }).toString();
    console.log("text",text);

    const res=await fetch("https://api.dropboxapi.com/oauth2/token", {
        "headers": {
            'Content-Type': 'application/x-www-form-urlencoded; charset=UTF-8',
        },
        "body":text,
        "method": "POST",
      });
    
        console.log(await res.json());


})();

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
