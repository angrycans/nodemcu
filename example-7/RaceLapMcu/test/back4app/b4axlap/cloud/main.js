Parse.Cloud.define("sendEmail", async (request) => {
  var API_KEY = "0c59ad5869a35dde3548aaeffc2dbda4-75cd784d-e7c8d672";
  var DOMAIN = "sandboxcb01f9c5709e44948318d103118cbb2e.mailgun.org";
  var mailgun = require("mailgun-js")({ apiKey: API_KEY, domain: DOMAIN });

  const data = {
    "from": "xlap@sandbox.mgsend.net",
    "to": request.params.toEmail,
    "subject": request.params.subject,
    "text": request.params.body,
  };

  console.log("sendEmail", data);

  try {
    const res = await mailgun.messages().send(data);
    console.log("res", res);
    return JSON.stringify(res);
  } catch (e) {
    return `{error:${e.error}}`;
  }
});

Parse.Cloud.define("refresh_dropbox_token", async (request) => {
  try {
    // Parse.initialize(
    //   "KgHFOTuWepLcBnpzrcZY0wXedW5LHhcY2i4gMzin",
    //   "3lCU1D8IrhdXqOvykqqVJWDUKzgWJOOR2c4CUe0Z",
    // );
    // Parse.serverURL = "https://parseapi.back4app.com/";
    const fetch = require("node-fetch");

    const text = new URLSearchParams({
      "grant_type": "refresh_token",
      "client_id": "054fu81io7bpqjm",
      "client_secret": "fkjvl5t718xyj16",
      "refresh_token":
        "7n_Dt3I2izUAAAAAAAAAAbno9CplSr8CcdFYeMkA-1-Uuf-FX2fWks0ep8Xx7npL",
    }).toString();
    console.log("text", text);

    const res = await fetch("https://api.dropboxapi.com/oauth2/token", {
      "headers": {
        "Content-Type": "application/x-www-form-urlencoded; charset=UTF-8",
      },
      "body": text,
      "method": "POST",
    });

    const ret = await res.json();
    console.log("ret", ret);

    const q = new Parse.Query("Dropbox_token");

    q.equalTo("name", "access_token");
    const qret = await q.first();

    console.log("results", qret);

    if (qret) {
      qret.set("token", ret.access_token);
      await qret.save();
      console.log("results", qret);
    } else {
      const Dropbox_token = Parse.Object.extend("Dropbox_token");
      const token = new Dropbox_token();
      token.set("name", "access_token");
      token.set("token", ret.access_token);
      let token_rst = await token.save();
      console.log("token_rst", token_rst);
    }

    return (JSON.stringify(ret));
  } catch (e) {
    console.log("catch error ", e);
    return `{error:${e.error}}`;
  }
});

Parse.Cloud.job("refresh_dropbox_token_job", async (request) => {
  try {
    const fetch = require("node-fetch");

    const text = new URLSearchParams({
      "grant_type": "refresh_token",
      "client_id": "054fu81io7bpqjm",
      "client_secret": "fkjvl5t718xyj16",
      "refresh_token":
        "7n_Dt3I2izUAAAAAAAAAAbno9CplSr8CcdFYeMkA-1-Uuf-FX2fWks0ep8Xx7npL",
    }).toString();
    console.log("refresh_dropbox_token jobtext", text);

    const res = await fetch("https://api.dropboxapi.com/oauth2/token", {
      "headers": {
        "Content-Type": "application/x-www-form-urlencoded; charset=UTF-8",
      },
      "body": text,
      "method": "POST",
    });

    const ret = await res.json();
    console.log("JOB refresh_dropbox_token  ret", ret);

    const q = new Parse.Query("Dropbox_token");

    q.equalTo("name", "access_token");
    const qret = await q.first();

    console.log("JOB results", qret);

    if (qret) {
      qret.set("token", ret.access_token);
      await qret.save();
      console.log("JOB results", qret);
    } else {
      const Dropbox_token = Parse.Object.extend("Dropbox_token");
      const token = new Dropbox_token();
      token.set("name", "access_token");
      token.set("token", ret.access_token);
      let token_rst = await token.save();
      console.log("JOB token_rst ", token_rst);
    }

    return (JSON.stringify(ret));
  } catch (e) {
    console.log("JOB catch error ", e);
    return `{JOB refresh_dropbox_token   error:${e.error}}`;
  }
});
