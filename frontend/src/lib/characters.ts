
export interface CharacterData {
    name: Character,
    initials: string,
}

export enum Character
{
    Whale = 0,
    Horse,
    TwoFace,
    // Pumpkin,
    // Dino,
    Hamburger,
    // Shark,
    MonaLisa,
    Fish,
    Butterfly,
    Turkey,
    UpsideDown,
    Robot,
    Frog,
    Superhero,
    Dad
};

export let character_data: CharacterData[] = [
    {name: Character.Whale, initials: "🐳"},
    {name: Character.Horse, initials: "🐴"},
    {name: Character.TwoFace, initials: "👨‍👧"},
    // {name: Character.Pumpkin, initials: "🎃"},
    // {name: Character.Dino, initials: "🦖"},
    {name: Character.Hamburger, initials: "🍔"},
    // {name: Character.Shark, initials: "🦈"}, 
    {name: Character.MonaLisa, initials: "🖼️"},
    {name: Character.Fish, initials: "🐟"},
    {name: Character.Frog, initials: "🐸"},
    {name: Character.Butterfly, initials: "🦋"},
    {name: Character.Turkey, initials: "🦃"},
    {name: Character.UpsideDown, initials: "🙃"},
    {name: Character.Robot, initials: "🤖"},
    {name: Character.Frog, initials: "🐸"},
    {name: Character.Superhero, initials: "🦸"},
    {name: Character.Dad, initials: "👨"},
];
