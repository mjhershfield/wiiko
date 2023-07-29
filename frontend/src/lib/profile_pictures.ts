
export interface ProfilePictureData {
    name: ProfilePicture,
    initials: string,
}

export enum ProfilePicture
{
    Jesus,
    Whale,
    Horse,
    TwoFace,
    Pumpkin,
    Dino,
    Hamburger,
    Shark,
    MonaLisa,
    Panda,
    Fish,
    Frog,
    Butterfly,
    Basketball,
    Owl,
    Spaghetti,
    Turkey,
    Dragon,
    UpsideDown,
    None,
};

export let profile_picture_data: ProfilePictureData[] = [
    {name: ProfilePicture.Jesus, initials: "✝️"}, 
    {name: ProfilePicture.Whale, initials: "🐳"},
    {name: ProfilePicture.Horse, initials: "🐴"},
    {name: ProfilePicture.TwoFace, initials: "👨‍👧"},
    {name: ProfilePicture.Pumpkin, initials: "🎃"},
    {name: ProfilePicture.Dino, initials: "🦖"},
    {name: ProfilePicture.Hamburger, initials: "🍔"},
    {name: ProfilePicture.Shark, initials: "🦈"}, 
    {name: ProfilePicture.MonaLisa, initials: "🖼️"},
    {name: ProfilePicture.Panda, initials: "🐼"},
    {name: ProfilePicture.Fish, initials: "🐟"},
    {name: ProfilePicture.Frog, initials: "🐸"},
    {name: ProfilePicture.Butterfly, initials: "🦋"},
    {name: ProfilePicture.Basketball, initials: "🏀"},
    {name: ProfilePicture.Owl, initials: "🦉"}, 
    {name: ProfilePicture.Spaghetti, initials: "🍝"},
    {name: ProfilePicture.Turkey, initials: "🦃"},
    {name: ProfilePicture.Dragon, initials: "🐲"},
    {name: ProfilePicture.UpsideDown, initials: "🙃"}
];
