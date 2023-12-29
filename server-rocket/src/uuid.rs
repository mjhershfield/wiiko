use std::str::FromStr;

use rocket::{request::{FromRequest, Outcome}, Request, http::Status, form::FromFormField};
use uuid::Uuid;

pub struct ClientUuid(Uuid);

#[derive(Debug)]
pub enum ClientUuidError {
    NoUuid,
    InvalidUuid
}

#[rocket::async_trait]
impl<'r> FromRequest<'r> for ClientUuid {
    type Error = ClientUuidError;

    async fn from_request(req: &'r Request<'_>) -> Outcome<Self, Self::Error> {

        match req.headers().get_one("x-client-uuid") {
            None => Outcome::Error((Status::BadRequest, ClientUuidError::NoUuid)),
            Some(uuid_string) => {
                match Uuid::from_str(uuid_string) {
                    Ok(uuid) => Outcome::Success(ClientUuid(uuid)),
                    Err(_) => Outcome::Error((Status::BadRequest, ClientUuidError::InvalidUuid))
                }
            } 
        }
    }
}

#[rocket::async_trait]
impl<'v> FromFormField<'v> for ClientUuid {
    fn from_value(field:rocket::form::ValueField<'v>) -> rocket::form::Result<'v,Self> {
        match Uuid::from_str(field.value) {
            Ok(uuid) => Ok(ClientUuid(uuid)),
            Err(_) => Err(rocket::form::Error::validation("Invalid UUID param"))?
        }
    }
}

impl Into<Uuid> for ClientUuid {
    fn into(self) -> Uuid {
        self.0
    }
}