
async function main() {
    const PAYLOAD = window.workingDir + '/devilutionx.elf';

    return {
        mainText: "DevilutionX",
        secondaryText: 'Diablo build for modern OSes',
        onclick: async () => {
	    return {
		path: PAYLOAD,
		args: ''
	    };
        },
	options: [
	    {
		text: "Skip Intro",
		onclick: async () => {
		    return {
			path: PAYLOAD,
			args: '-n'
		    };
		}
        }
	]
    };
}
