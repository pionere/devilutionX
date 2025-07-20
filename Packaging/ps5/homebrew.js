
async function main() {
	const PAYLOAD = window.workingDir + '/devilutionx';
	const PATCHER = window.workingDir + '/devil_patcher';

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
		},
		{
		text: "Run Patcher",
		onclick: async () => {
			return {
				path: PATCHER,
				args: ''
			};
		}
		}
	]
	};
}
