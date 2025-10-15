const axios = require('axios');

exports.handler = async function (context, event, callback) {
  const records = await getData(context, event)

  const found = records.length > 0;

  const twiml = new Twilio.twiml.VoiceResponse();

  if (found) {
    const name = records[0].fields.Name;
    twiml.say(`Hello ${name}. Your phone number is authorized. Welcome!`);

    await axios.post(context.ENDPOINT_URL, records[0].fields, {
      headers: {
        "X-API-Key": context.ENDPOINT_API_KEY,
      },
      timeout: 10000, // optional
    });
  } else {
    twiml.say(`Sorry, this phone number is not authorized.`);
  }

  return callback(null, twiml);

};


async function getData(context, event) {
  const phone = event.From;

  const url = `https://api.airtable.com/v0/${context.AIRTABLE_BASE_ID}/${context.AIRTABLE_TABLE}`;
  const params = { filterByFormula: `{Phone}='${phone}'` };

  const response = await axios.get(url, {
    params,
    headers: { Authorization: `Bearer ${context.AIRTABLE_TOKEN}` },
  });

  const records = response.data.records;

  return records
}