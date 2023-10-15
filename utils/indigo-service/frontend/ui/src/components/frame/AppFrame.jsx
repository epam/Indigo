import 'ketcher-react/dist/index.css'
import './AppFrame.scss';
import {Editor} from "ketcher-react";
import {StandaloneStructServiceProvider} from "ketcher-standalone";

const getHiddenButtonsConfig = () => {
    const searchParams = new URLSearchParams(window.location.search)
    const hiddenButtons = searchParams.get('hiddenControls')

    if (!hiddenButtons) return {}

    return hiddenButtons.split(',').reduce((acc, button) => {
        if (button) acc[button] = {hidden: true}

        return acc
    }, {})
}


const AppFrame = () => {
    const hiddenButtonsConfig = getHiddenButtonsConfig()

    return (
        <>
            <div className={"editor"}>
                <Editor
                    errorHandler={() => {
                    }}
                    buttons={hiddenButtonsConfig}
                    staticResourcesUrl={process.env.PUBLIC_URL}
                    structServiceProvider={new StandaloneStructServiceProvider()}
                    onInit={(ketcher) => {
                        ;(global).ketcher = ketcher
                        window.parent.postMessage(
                            {
                                eventType: 'init'
                            }, '*'
                        )
                    }}
                />
            </div>
        </>
    )
};

export default AppFrame;
